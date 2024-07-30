/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tripgroupmanager.h"
#include "tripgroup.h"
#include "constants.h"
#include "logging.h"
#include "reservationmanager.h"
#include "transfermanager.h"

#include <KItinerary/Event>
#include <KItinerary/LocationUtil>
#include <KItinerary/Organization>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>

#include <KLocalizedString>

#include <QDateTime>
#include <QDebug>
#include <QDirIterator>
#include <QStandardPaths>
#include <QUuid>

#include <set>

using namespace Qt::Literals::StringLiterals;
using namespace KItinerary;

constexpr inline const auto MaximumTripDuration = 20; // in days
constexpr inline const auto MaximumTripElements = 30;

TripGroupManager::TripGroupManager(QObject* parent) :
    QObject(parent)
{
    load();
}

TripGroupManager::~TripGroupManager() = default;

ReservationManager* TripGroupManager::reservationManager() const
{
    return m_resMgr;
}

void TripGroupManager::setReservationManager(ReservationManager *resMgr)
{
    m_resMgr = resMgr;
    connect(m_resMgr, &ReservationManager::batchAdded, this, &TripGroupManager::batchAdded);
    connect(m_resMgr, &ReservationManager::batchContentChanged, this, &TripGroupManager::batchContentChanged);
    connect(m_resMgr, &ReservationManager::batchRemoved, this, &TripGroupManager::batchRemoved);
    connect(m_resMgr, &ReservationManager::batchRenamed, this, &TripGroupManager::batchRenamed);

    const auto allReservations = m_resMgr->batches();
    m_reservations.clear();
    m_reservations.reserve(allReservations.size());
    std::copy(allReservations.begin(), allReservations.end(), std::back_inserter(m_reservations));
    std::sort(m_reservations.begin(), m_reservations.end(), [this](const auto &lhs, const auto &rhs) {
        return SortUtil::isBefore(m_resMgr->reservation(lhs), m_resMgr->reservation(rhs));
    });

    if (m_resMgr && m_transferMgr) {
        checkConsistency();
        scanAll();
    }
}

void TripGroupManager::setTransferManager(TransferManager *transferMgr)
{
    m_transferMgr = transferMgr;

    const auto transferChangedWrapper = [this](const Transfer &t) { transferChanged(t.reservationId(), t.alignment()); };
    connect(m_transferMgr, &TransferManager::transferAdded, this, transferChangedWrapper);
    connect(m_transferMgr, &TransferManager::transferChanged, this, transferChangedWrapper);
    connect(m_transferMgr, &TransferManager::transferRemoved, this, &TripGroupManager::transferChanged);

    if (m_resMgr && m_transferMgr) {
        checkConsistency();
        scanAll();
    }
}

void TripGroupManager::suspend()
{
    m_suspended = true;
    m_shouldScan = false;
}

void TripGroupManager::resume()
{
    m_suspended = false;
    if (m_shouldScan) {
        scanAll();
    }
}

std::vector<QString> TripGroupManager::tripGroups() const
{
    std::vector<QString> groups;
    groups.reserve(m_tripGroups.size());
    std::copy(m_tripGroups.keyBegin(), m_tripGroups.keyEnd(), std::back_inserter(groups));
    return groups;
}

TripGroup TripGroupManager::tripGroup(const QString &id) const
{
    return m_tripGroups.value(id);
}

QString TripGroupManager::tripGroupIdForReservation(const QString &resId) const
{
    return m_reservationToGroupMap.value(resId);
}

TripGroup TripGroupManager::tripGroupForReservation(const QString &resId) const
{
    return tripGroup(m_reservationToGroupMap.value(resId));
}

QString TripGroupManager::basePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tripgroups/"_L1;
}

QString TripGroupManager::fileForGroup(QStringView tgId)
{
    return basePath() + tgId + ".json"_L1;
}

void TripGroupManager::load()
{
    const auto base = basePath();
    QDir::root().mkpath(base);

    for (QDirIterator it(base, QDir::NoDotAndDotDot | QDir::Files); it.hasNext();) {
        it.next();
        TripGroup g;
        if (g.load(it.filePath())) {
            const auto tgId = it.fileInfo().baseName();
            m_tripGroups.insert(tgId, g);
            for (const auto &resId : g.elements()) {
                const auto groupIt = m_reservationToGroupMap.constFind(resId);
                if (groupIt != m_reservationToGroupMap.constEnd()) {
                    qCWarning(Log) << "Overlapping trip groups found - removing" << g.name();
                    const auto groupId = groupIt.value(); // copy before we modify what groupIt points to - NOLINT performance-unnecessary-copy-initialization
                    removeTripGroup(groupId);
                    removeTripGroup(tgId);
                    break;
                }
                m_reservationToGroupMap.insert(resId, tgId);
            }
        }
    }
}

void TripGroupManager::removeTripGroup(const QString &groupId)
{
    const auto groupIt = m_tripGroups.constFind(groupId);
    if (groupIt == m_tripGroups.constEnd()) {
        return;
    }

    for (const auto &elem : groupIt.value().elements()) {
        const auto it = m_reservationToGroupMap.find(elem);
        // check if this still points to the removed group (might not be the case if an overlapping group was added meanwhile)
        if (it != m_reservationToGroupMap.end() && it.value() == groupId) {
            m_reservationToGroupMap.erase(it);
        }
    }
    m_tripGroups.erase(groupIt);
    if (!QFile::remove(fileForGroup(groupId))) {
        qCWarning(Log) << "Failed to delete trip group file!" << groupId;
    }
    Q_EMIT tripGroupRemoved(groupId);
}

void TripGroupManager::clear()
{
    qCDebug(Log) << "deleting" << basePath();
    QDir d(basePath());
    d.removeRecursively();
}

void TripGroupManager::updateTripGroup(const QString &groupId, const TripGroup &group)
{
    auto groupIt = m_tripGroups.find(groupId);
    if (groupIt == m_tripGroups.end()) {
        return;
    }

    groupIt.value() = group;
    recomputeTripGroupTimes(groupIt.value());
    groupIt.value().store(fileForGroup(groupId));
    Q_EMIT tripGroupChanged(groupId);
}

void TripGroupManager::removeReservationsInGroup(const QString &groupId)
{
    const auto groupIt = m_tripGroups.constFind(groupId);
    if (groupIt == m_tripGroups.constEnd()) {
        return;
    }

    // as we remove entries one by one we'd get and propagate update notifications for all of them
    // so block those and emit one notifications manually once we are done
    TripGroupingBlocker groupingBlocker(this);
    QSignalBlocker signalBlocker(this);

    const auto elements = groupIt.value().elements();
    for (const auto &element : elements) {
        m_resMgr->removeBatch(element);
    }

    signalBlocker.unblock();
    Q_EMIT tripGroupRemoved(groupId);
}

void TripGroupManager::batchAdded(const QString &resId)
{
    auto it = std::lower_bound(m_reservations.begin(), m_reservations.end(), resId, [this](const auto &lhs, const auto &rhs) {
        return SortUtil::isBefore(m_resMgr->reservation(lhs), m_resMgr->reservation(rhs));
    });
    m_reservations.insert(it, resId);
    // ### we can optimize this by only scanning it +/- MaximumTripElements
    scanAll();
}

void TripGroupManager::batchContentChanged(const QString &resId)
{
    const auto tgId = tripGroupIdForReservation(resId);
    auto tg = tripGroup(tgId);
    if (!tg.isAutomaticallyGrouped()) {
        // don't touch the grouping, just deal with potential time changes
        auto elements = tg.elements();
        std::sort(elements.begin(), elements.end(), [this](const auto &lhs, const auto &rhs) {
            return SortUtil::isBefore(m_resMgr->reservation(lhs), m_resMgr->reservation(rhs));
        });
        tg.setElements(elements);
        recomputeTripGroupTimes(tg);
        tg.store(fileForGroup(tgId));
        Q_EMIT tripGroupChanged(tgId);
    } else {
        // ### we can probably make this more efficient
        batchRemoved(resId);
        batchAdded(resId);
    }
}

void TripGroupManager::batchRenamed(const QString &oldBatchId, const QString &newBatchId)
{
    const auto mapIt = m_reservationToGroupMap.constFind(oldBatchId);
    if (mapIt == m_reservationToGroupMap.end()) {
        return;
    }

    const auto tgId = mapIt.value(); // NOLINT performance-unnecessary-copy-initialization

    auto tg = tripGroup(tgId);
    auto elems = tg.elements();
    const auto elemIt = std::find(elems.begin(), elems.end(), oldBatchId);
    (*elemIt) = newBatchId;
    tg.setElements(elems);

    m_reservationToGroupMap.erase(mapIt);
    m_reservationToGroupMap[newBatchId] = tgId;

    tg.store(fileForGroup(tgId));
    Q_EMIT tripGroupChanged(tgId);
}

void TripGroupManager::batchRemoved(const QString &resId)
{
    // check if resId is part of a group
    const auto mapIt = m_reservationToGroupMap.constFind(resId);
    if (mapIt != m_reservationToGroupMap.constEnd()) {
        const auto groupIt = m_tripGroups.find(mapIt.value());
        Q_ASSERT(groupIt != m_tripGroups.end());
        const auto groupId = groupIt.key(); // copy as the iterator might become invalid below - NOLINT performance-unnecessary-copy-initialization

        auto elems = groupIt.value().elements();
        elems.removeAll(resId);
        if (elems.isEmpty()) { // group deleted
            qDebug() << "removing empty trip group";
            removeTripGroup(groupId);
        } else { // group changed
            qDebug() << "removing element from trip group" << resId << elems;
            groupIt.value().setElements(elems);
            recomputeTripGroupTimes(groupIt.value());
            groupIt.value().store(fileForGroup(mapIt.value()));
            m_reservationToGroupMap.erase(mapIt);
            Q_EMIT tripGroupChanged(groupId);
        }
    }

    // remove the reservation
    const auto resIt = std::find(m_reservations.begin(), m_reservations.end(), resId);
    if (resIt != m_reservations.end()) {
        m_reservations.erase(resIt);
    }
}

void TripGroupManager::transferChanged(const QString &resId, Transfer::Alignment alignment)
{
    const auto tgId = m_reservationToGroupMap.value(resId);
    if (tgId.isEmpty()) {
        return;
    }

    const auto tgIt = m_tripGroups.find(tgId);
    assert(tgIt != m_tripGroups.end() && !tgIt.value().elements().empty());

    // if tranfers leading or trailing the trip group changed, the start/end time of the group change
    if ((alignment == Transfer::Before && tgIt.value().elements().constFirst() == resId)
     || (alignment == Transfer::After && tgIt.value().elements().constLast() == resId)) {
        if (recomputeTripGroupTimes(tgIt.value())) {
            tgIt.value().store(fileForGroup(tgIt.key()));
            Q_EMIT tripGroupChanged(tgId);
        }
    }
}

void TripGroupManager::scanAll()
{
    if (m_suspended) {
        m_shouldScan = true;
        return;
    }

    qCDebug(Log);
    QString prevGroup;
    for (auto it = m_reservations.begin(); it != m_reservations.end(); ++it) {
        auto groupIt = m_reservationToGroupMap.constFind(*it);
        if (groupIt != m_reservationToGroupMap.constEnd() && groupIt.value() == prevGroup) {
            // in the middle of an existing group
            continue;
        }

        if (groupIt == m_reservationToGroupMap.constEnd()) {
            prevGroup.clear();
        } else {
            prevGroup = groupIt.value();
        }

        scanOne(it);
        prevGroup = m_reservationToGroupMap.value(*it);
    }
}

static bool isConnectedTransition(const QVariant &fromRes, const QVariant &toRes)
{
    const auto from = LocationUtil::arrivalLocation(fromRes);
    const auto to = LocationUtil::isLocationChange(toRes) ? LocationUtil::departureLocation(toRes) :LocationUtil::location(toRes);
    if (LocationUtil::isSameLocation(from, to, LocationUtil::CityLevel)) {
        return true;
    }

    const auto dep = SortUtil::endDateTime(fromRes);
    const auto arr = SortUtil::startDateTime(toRes);
    return dep.date() == arr.date() && dep.secsTo(arr) < Constants::MaximumLayoverTime.count();
}

void TripGroupManager::scanOne(std::vector<QString>::const_iterator beginIt)
{
    const auto beginRes = m_resMgr->reservation(*beginIt);
    const auto beginDeparture = LocationUtil::departureLocation(beginRes);
    const auto beginDt = SortUtil::startDateTime(beginRes);

    m_resNumSearch.clear();
    if (JsonLd::canConvert<Reservation>(beginRes)) {
        m_resNumSearch.push_back({beginRes.userType(), JsonLd::convert<Reservation>(beginRes).reservationNumber()});
    }

    const auto prevTgId = tripGroupIdForReservation(*beginIt);
    const auto prevTg = tripGroup(prevTgId);
    const auto explicitEnd = !prevTg.isAutomaticallyGrouped() && !prevTg.elements().empty() ? prevTg.elements().constLast() : QString();

    qDebug() << "starting scan at" << LocationUtil::name(beginDeparture);
    auto res = beginRes;
    auto resNumIt = m_reservations.cend(); // result of the search using reservation ids
    auto connectedIt = m_reservations.cend(); // result of the search using trip connectivity
    auto explicitIt = m_reservations.cend(); // result of the search using an existing explicitly managed group

    bool resNumSearchDone = false;
    bool connectedSearchDone = false;
    bool explicitSearchDone = explicitEnd.isEmpty();
    bool reachedStartAgain = false;

    // scan by location change
    for (auto it = beginIt + 1; it != m_reservations.end(); ++it) {
        // if we reached the next manually grouped element here, stop immediately
        if (const auto tgId = tripGroupIdForReservation(*it); !tgId.isEmpty() && tgId != prevTgId && !tripGroup(tgId).isAutomaticallyGrouped()) {
            break;
        }

        const auto prevRes = res;
        const auto curRes = m_resMgr->reservation(*it);
        const auto isLocationChange = LocationUtil::isLocationChange(curRes);

        // not a location change? -> continue searching
        if (isLocationChange) {
            res = curRes;
        }

        // all search strategies think they are done
        if (resNumSearchDone && connectedSearchDone && explicitSearchDone) {
            break;
        }

        // search depth reached
        // ### we probably don't want to count multi-traveler elements for this!
        if (explicitSearchDone && std::distance(beginIt, it) > MaximumTripElements) {
            qDebug() << "  aborting search, maximum search depth reached";
            break;
        }

        // maximum trip duration exceeded?
        const auto endDt = SortUtil::endDateTime(curRes);
        if (explicitSearchDone && beginDt.daysTo(endDt) > MaximumTripDuration) {
            qDebug() << "  aborting search, maximum trip duration reached";
            break;
        }

        // check for connected transitions (ie. previous arrival == current departure)
        const auto prevArrival = LocationUtil::arrivalLocation(prevRes);
        const auto curDeparture = isLocationChange ? LocationUtil::departureLocation(curRes) :LocationUtil::location(curRes);
        const auto connectedTransition = isConnectedTransition(prevRes, curRes);
        qDebug() << "  current transition goes from" << LocationUtil::name(prevArrival) << "to" << (isLocationChange ? LocationUtil::name(LocationUtil::arrivalLocation(curRes)) : LocationUtil::name(curDeparture)) << connectedTransition;

        if (!connectedSearchDone) {
            if (!connectedTransition && isLocationChange) {
                qDebug() << "  aborting connectivity search, not an adjacent transition from" << LocationUtil::name(prevArrival) << "to" << LocationUtil::name(curDeparture);
                connectedSearchDone = true;
            }
            if (connectedTransition) {
                connectedIt = it;
            }

            // same location as beginIt? -> we reached the end of the trip (break)
            if (isLocationChange) {
                const auto curArrival = LocationUtil::arrivalLocation(curRes);
                if (LocationUtil::isSameLocation(beginDeparture, curArrival, LocationUtil::CityLevel)) {
                    qDebug() << "  aborting connectivity search, arrived at the start again" << LocationUtil::name(curArrival);
                    connectedSearchDone = true;
                    reachedStartAgain = true;
                }
            }
        }

        if (isLocationChange && !resNumSearchDone &&  JsonLd::canConvert<Reservation>(curRes)) {
            const auto resNum = JsonLd::convert<Reservation>(curRes).reservationNumber();
            if (!resNum.isEmpty()) {
                const auto r = std::find_if(m_resNumSearch.begin(), m_resNumSearch.end(), [curRes, resNum](const auto &elem) {
                    return elem.type == curRes.userType() && elem.resNum == resNum;
                });
                if (r == m_resNumSearch.end()) {
                    // mode of transport or reservation changed: we consider this still part of the trip if connectivity
                    // search thinks this is part of the same trip too, and we are not at home again yet
                    if (connectedTransition && !LocationUtil::isSameLocation(prevArrival, beginDeparture, LocationUtil::CityLevel)) {
                        qDebug() << "  considering transition to" << LocationUtil::name(LocationUtil::arrivalLocation(curRes)) << "as part of trip despite unknown reservation number";
                        m_resNumSearch.push_back({curRes.userType(), resNum});
                        resNumIt = it;
                    } else {
                        qDebug() << "  aborting reservation number search due to mismatch";
                        resNumSearchDone = true;
                    }
                } else {
                    if (reachedStartAgain) {
                        qDebug() << "    continuing due to matching reservation number" << resNum;
                    }
                    resNumIt = it;
                }
            } else if (reachedStartAgain) {
                // when the next entry has no reservation number and we already found a loop with connectivity search
                // consider that good enough and stop here
                qDebug() << "   aborting reservation number search due to no reservation number on the next element";
                resNumSearchDone = true;
            }
        }

        if (!explicitSearchDone && (*it) == explicitEnd) {
            explicitIt = it;
            explicitSearchDone = true;
        }
    }

    // determine which search strategy found the larger result
    auto it = m_reservations.cend();
    if (connectedIt != m_reservations.end() && resNumIt != m_reservations.end()) {
        it = std::max(connectedIt, resNumIt);
    } else {
        it = connectedIt == m_reservations.end() ? resNumIt : connectedIt;
    }
    if (explicitIt != m_reservations.cend()) {
        it = it == m_reservations.end() ? explicitIt : std::max(it, explicitIt);
    }

    if (it == m_reservations.end()) {
        qDebug() << "nothing found";
        createAutomaticGroup({*beginIt});
        return;
    }

    // remove leading loop appendices (trailing ones will be cut by the loop check above already)
    if (prevTg.isAutomaticallyGrouped()) {
        const auto endRes = m_resMgr->reservation(*it);
        const auto endArrival = LocationUtil::arrivalLocation(endRes);
        for (auto it2 = beginIt; it2 != it; ++it2) {
            const auto res = m_resMgr->reservation(*it2);
            if (!LocationUtil::isLocationChange(res)) {
                continue;
            }
            const auto curDeparture = LocationUtil::departureLocation(res);
            if (LocationUtil::isSameLocation(endArrival, curDeparture, LocationUtil::CityLevel)) {
                if (beginIt != it2) {
                    qDebug() << "  removing leading appendix, starting at" << LocationUtil::name(curDeparture);
                    QStringList appendixElems;
                    appendixElems.reserve(std::distance(beginIt, it2));
                    std::copy(beginIt, it2, std::back_inserter(appendixElems));
                    createAutomaticGroup(appendixElems);
                }
                beginIt = it2;
                break;
            }
        }
    }

    if (beginIt == it) {
        qDebug() << "found empty trip";
        createAutomaticGroup({*beginIt});
        return;
    }

    // create a trip for [beginIt, it)
    ++it; // so this marks the end
    QList<QString> elems;
    elems.reserve(std::distance(beginIt, it));
    std::copy(beginIt, it, std::back_inserter(elems));
    createAutomaticGroup(elems);
}

void TripGroupManager::createAutomaticGroup(const QStringList &elems)
{
    Q_ASSERT(!elems.empty());

    const auto groupIt = m_tripGroups.find(m_reservationToGroupMap.value(elems.front()));
    if (groupIt != m_tripGroups.end() && groupIt.value().elements() == elems) {
        qDebug() << "existing group unchanged" << groupIt.value().name();
        return;
    }

    std::set<QString> pendingGroupRemovals;
    if (groupIt == m_tripGroups.end()) {
        const auto tgId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        TripGroup g;
        g.setElements(elems);
        for (const auto &elem : elems) {
            // remove overlapping/nested groups, delay this until the end though, as that will invalidate our iterators
            const auto previousGroupId = m_reservationToGroupMap.value(elem);
            if (!previousGroupId.isEmpty() && previousGroupId != tgId) {
                pendingGroupRemovals.insert(previousGroupId);
            }
            m_reservationToGroupMap.insert(elem, tgId);
        }
        g.setName(guessName(g.elements()));
        recomputeTripGroupTimes(g);
        qDebug() << "creating trip group" << g.name();
        m_tripGroups.insert(tgId, g);
        g.store(fileForGroup(tgId));
        Q_EMIT tripGroupAdded(tgId);
    } else {
        auto &g = groupIt.value();
        for (const auto &elem : g.elements()) { // remove old element mappings, some of them might no longer be valid
            m_reservationToGroupMap.remove(elem);
        }
        g.setElements(elems);
        for (const auto &elem : elems) {
            m_reservationToGroupMap.insert(elem, groupIt.key());
        }
        if (g.hasAutomaticName() || g.name().isEmpty()) {
            g.setName(guessName(g.elements()));
        }
        recomputeTripGroupTimes(g);
        qDebug() << "updating trip group" << g.name();
        g.store(fileForGroup(groupIt.key()));
        Q_EMIT tripGroupChanged(groupIt.key());
    }

    for (const auto &tgId : pendingGroupRemovals) {
        removeTripGroup(tgId);
    }
}

void TripGroupManager::checkConsistency()
{
    std::vector<QString> tgIds;
    tgIds.reserve(m_tripGroups.size());

    // look for dangling reservation references
    for (auto it = m_reservationToGroupMap.constBegin(); it != m_reservationToGroupMap.constEnd(); ++it) {
        if (!m_resMgr->hasBatch(it.key())) {
            tgIds.push_back(it.value());
        }
    }

    for (const auto &groupId : tgIds) {
        qCWarning(Log) << "Removing group" << m_tripGroups.value(groupId).name() << "with dangling reservation references";
        removeTripGroup(groupId);
    }
    tgIds.clear();

    // look for missing or otherwise corrupt begin/end times
    for (auto it = m_tripGroups.begin(); it != m_tripGroups.end(); ++it) {
        if (it.value().beginDateTime().isValid() && it.value().endDateTime().isValid() && it.value().beginDateTime() <= it.value().endDateTime()) {
            continue;
        }
        qCDebug(Log) << "Fixing begin/end times for group" << it.value().name();
        recomputeTripGroupTimes(it.value());
        it.value().store(fileForGroup(it.key()));
    }

    // look for nested groups
    std::copy(m_tripGroups.keyBegin(), m_tripGroups.keyEnd(), std::back_inserter(tgIds));
    std::sort(tgIds.begin(), tgIds.end(), [this](const auto &lhs, const auto &rhs) {
        return m_tripGroups.value(lhs).beginDateTime() < m_tripGroups.value(rhs).beginDateTime();
    });
    for (auto it = tgIds.begin();;) {
        it = std::adjacent_find(it, tgIds.end(), [this](const auto &lhs, const auto &rhs) {
            return m_tripGroups.value(lhs).endDateTime() > m_tripGroups.value(rhs).beginDateTime();
        });
        if (it == tgIds.end()) {
            break;
        }
        // remove both nested groups
        qCWarning(Log) << "Removing group" << m_tripGroups.value(*it).name() << "due to overlapping with following group";
        it = tgIds.erase(it);
        qCWarning(Log) << "Removing group" << m_tripGroups.value(*it).name() << "due to overlapping with previous group";
        it = tgIds.erase(it);
    }
}

static QString destinationName(const QVariant &loc)
{
    const auto addr = LocationUtil::address(loc);
    if (!addr.addressLocality().isEmpty()) {
        return addr.addressLocality();
    }
    return LocationUtil::name(loc);
}

QString TripGroupManager::guessDestinationFromEvent(const QStringList &elements) const
{
    // compute overall trip length
    const auto beginDt = SortUtil::startDateTime(m_resMgr->reservation(elements.front()));
    const auto endDt = SortUtil::endDateTime(m_resMgr->reservation(elements.back()));
    if (!beginDt.isValid() || !endDt.isValid()) {
        return {};
    }
    const auto tripLength = beginDt.secsTo(endDt);

    // find an event that covers 50+% of the trip time
    for (const auto &elem : elements) {
        const auto res = m_resMgr->reservation(elem);
        if (!JsonLd::isA<EventReservation>(res)) {
            continue;
        }

        const auto evBeginDt = SortUtil::startDateTime(res);
        const auto evEndDt = SortUtil::endDateTime(res);
        if (!evBeginDt.isValid() || !evEndDt.isValid()) {
            continue;
        }

        const auto eventLength = evBeginDt.secsTo(evEndDt);
        if (2 * eventLength > tripLength) {
            return res.value<EventReservation>().reservationFor().value<Event>().name();
        }
    }

    return {};
}

QString TripGroupManager::guessDestinationFromLodging(const QStringList &elements) const
{
    // we assume that lodging indicates the actual destination, not a stopover location
    QStringList dests;
    for (const auto &resId : elements) {
        const auto res = m_resMgr->reservation(resId);
        if (!JsonLd::isA<LodgingReservation>(res)) {
            continue;
        }

        const auto lodging = res.value<LodgingReservation>().reservationFor().value<LodgingBusiness>();
        if (!lodging.address().addressLocality().isEmpty() && !dests.contains(lodging.address().addressLocality())) {
            dests.push_back(lodging.address().addressLocality());
            continue;
        }
        if (!lodging.name().isEmpty() && !dests.contains(lodging.name())) { // fall back to hotel name if we don't know the city
            dests.push_back(lodging.name());
            continue;
        }

        // TODO consider the country if that differs from where we started from
    }

    return dests.join(" - "_L1);
}

bool TripGroupManager::isRoundTrip(const QStringList &elements) const
{
    const auto &depId = elements.at(0);
    const auto &arrId = elements.constLast();
    const auto dep = LocationUtil::departureLocation(m_resMgr->reservation(depId));
    const auto arr = LocationUtil::arrivalLocation(m_resMgr->reservation(arrId));
    return LocationUtil::isSameLocation(dep, arr, LocationUtil::CityLevel);
}

QString TripGroupManager::guessDestinationFromTransportTimeGap(const QStringList &elements) const
{
    // we must only do this for return trips
    if (!isRoundTrip(elements)) {
        return {};
    }

    // we assume that the largest time interval between arrival and departure of two adjacent location changes is the destination
    QDateTime beginDt;
    QString destName;
    qint64 maxLength = 0;

    for (const auto &resId : elements) {
        const auto res = m_resMgr->reservation(resId);
        if (!LocationUtil::isLocationChange(res)) {
            continue;
        }

        if (!beginDt.isValid()) { // first transport element
            beginDt = SortUtil::endDateTime(res);
            continue;
        }

        const auto endDt = SortUtil::startDateTime(res);
        const auto newLength = beginDt.secsTo(endDt);
        if (newLength > maxLength) {
            destName = LocationUtil::name(LocationUtil::departureLocation(res));
            maxLength = newLength;
        }
        beginDt = endDt;
    }

    return destName;
}

QVariant TripGroupManager::firstLocationChange(const QStringList &elements) const
{
    for (const auto &id : elements) {
        auto res = m_resMgr->reservation(id);
        if (LocationUtil::isLocationChange(res)) {
            return res;
        }
    }
    return {};
}

QVariant TripGroupManager::lastLocationChange(const QStringList &elements) const
{
    for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
        auto res = m_resMgr->reservation(*it);
        if (LocationUtil::isLocationChange(res)) {
            return res;
        }
    }
    return {};
}

QString TripGroupManager::guessName(const QStringList &elements) const
{
    if (elements.isEmpty()) {
        return {};
    }

    // part 1: the destination of the trip
    QString dest = guessDestinationFromEvent(elements);
    if (dest.isEmpty()) {
        dest = guessDestinationFromLodging(elements);
    }
    if (dest.isEmpty()) {
        dest = guessDestinationFromTransportTimeGap(elements);
    }
    if (dest.isEmpty()) {
        // two fallback cases: round-trips and one-way trips
        const auto beginLoc = LocationUtil::departureLocation(firstLocationChange(elements));
        const auto endLoc = LocationUtil::arrivalLocation(lastLocationChange(elements));
        if (LocationUtil::isSameLocation(beginLoc, endLoc, LocationUtil::CityLevel)) {
            const auto middleIdx = (elements.size() - 1 + (elements.size() % 2)) / 2;
            const auto middleRes = m_resMgr->reservation(elements.at(middleIdx));
            if (LocationUtil::isLocationChange(middleRes)) {
                dest = destinationName(LocationUtil::arrivalLocation(middleRes));
            } else {
                dest = destinationName(LocationUtil::location(middleRes));
            }
        } else {
            // TODO we want the city (or country, if differing from start) here, if available
            dest = destinationName(endLoc);
        }
    }
    // if none of the above worked, take anything we can find
    for (const auto &resId : elements) {
        if (!dest.isEmpty()) {
            break;
        }
        const auto res = m_resMgr->reservation(resId);
        dest = LocationUtil::name(LocationUtil::location(res));
        if (dest.isEmpty()) {
            dest = LocationUtil::name(LocationUtil::arrivalLocation(res));
        }
        if (dest.isEmpty() && JsonLd::isA<EventReservation>(res)) {
            dest = res.value<EventReservation>().reservationFor().value<Event>().name();
        }
        if (dest.isEmpty() && JsonLd::isA<LodgingReservation>(res)) {
            dest = res.value<EventReservation>().reservationFor().value<LodgingBusiness>().name();
        }
        if (dest.isEmpty()) {
            dest = LocationUtil::name(LocationUtil::departureLocation(res));
        }
    }


    // part 2: the time range of the trip
    // three cases: within 1 month, crossing a month boundary in one year, crossing a year boundary
    const auto beginDt = SortUtil::startDateTime(m_resMgr->reservation(elements.at(0)));
    const auto endDt = SortUtil::endDateTime(m_resMgr->reservation(elements.constLast()));
    if (beginDt.date().year() == endDt.date().year() || !endDt.isValid()) {
        if (beginDt.date().month() == endDt.date().month() || !endDt.isValid()) {
            return i18nc("%1 is destination, %2 is the standalone month name, %3 is the year", "%1 (%2 %3)", dest, QLocale().standaloneMonthName(beginDt.date().month(), QLocale::LongFormat), beginDt.date().toString(u"yyyy"));
        }
        return i18nc("%1 is destination, %2 and %3 are the standalone month names and %4 is the year", "%1 (%2/%3 %4)", dest, QLocale().monthName(beginDt.date().month(), QLocale::LongFormat), QLocale().standaloneMonthName(endDt.date().month(), QLocale::LongFormat), beginDt.date().toString(u"yyyy"));
    }
    return i18nc("%1 is destination, %2 and %3 are years", "%1 (%2/%3)", dest, beginDt.date().toString(u"yyyy"), endDt.date().toString(u"yyyy"));
}

bool TripGroupManager::recomputeTripGroupTimes(TripGroup &tg) const
{
    if (tg.elements().isEmpty() && (tg.beginDateTime().isValid() || tg.endDateTime().isValid())) {
        tg.setBeginDateTime({});
        tg.setEndDateTime({});
        return true;
    }
    if (tg.elements().isEmpty()) {
        return false;
    }

    auto res = m_resMgr->reservation(tg.elements().constFirst());
    auto dt = KItinerary::SortUtil::startDateTime(res);

    auto transfer = m_transferMgr->transfer(tg.elements().constFirst(), Transfer::Before);
    if (transfer.state() == Transfer::Selected && transfer.journey().scheduledDepartureTime().isValid()) {
        dt = std::min(dt, transfer.journey().scheduledDepartureTime());
    } else if (transfer.state() == Transfer::Pending && transfer.anchorTime().isValid()) {
        dt = std::min(dt, transfer.anchorTime().addSecs(-transfer.anchorTimeDelta()));
    }
    auto change = dt != tg.beginDateTime();
    tg.setBeginDateTime(dt);

    res = m_resMgr->reservation(tg.elements().constLast());
    dt = KItinerary::SortUtil::endDateTime(res);

    transfer = m_transferMgr->transfer(tg.elements().constLast(), Transfer::After);
    if (transfer.state() == Transfer::Selected && transfer.journey().scheduledArrivalTime().isValid()) {
        dt = std::max(dt, transfer.journey().scheduledArrivalTime());
    } else if (transfer.state() == Transfer::Pending && transfer.anchorTime().isValid()) {
        dt = std::min(dt, transfer.anchorTime().addSecs(transfer.anchorTimeDelta()));
    }
    change |= dt != tg.endDateTime();
    tg.setEndDateTime(dt);
    return change;
}

QString TripGroupManager::merge(const QString &tgId1, const QString &tgId2, const QString &newName)
{
    qCDebug(Log) << tgId1 << tgId2 << newName;

    const auto tg1 = tripGroup(tgId1);
    const auto tg2 = tripGroup(tgId2);

    QStringList elements;
    elements.reserve(tg1.elements().size() + tg2.elements().size());
    elements.append(tg1.elements());
    elements.append(tg2.elements());
    std::sort(elements.begin(), elements.end(), [this](const auto &lhs, const auto &rhs) {
        return SortUtil::isBefore(m_resMgr->reservation(lhs), m_resMgr->reservation(rhs));
    });

    for (const auto &id : elements) {
        m_reservationToGroupMap[id] = tgId1;
    }

    TripGroup group;
    group.setElements(elements);
    group.setIsAutomaticallyGrouped(false);

    group.setName(guessName(elements));
    if (group.name() != newName && !newName.isEmpty()) {
        group.setName(newName);
        group.setNameIsAutomatic(false);
    }

    group.setMatrixRoomId(tg1.matrixRoomId().isEmpty() ? tg2.matrixRoomId() : tg1.matrixRoomId());

    recomputeTripGroupTimes(group);
    m_tripGroups[tgId1] = group;
    group.store(fileForGroup(tgId1));

    m_tripGroups.remove(tgId2);
    if (!QFile::remove(fileForGroup(tgId2))) {
        qCWarning(Log) << "Failed to delete trip group file!" << tgId2;
    }

    Q_EMIT tripGroupRemoved(tgId2);
    Q_EMIT tripGroupChanged(tgId1);

    return tgId1;
}

QString TripGroupManager::createGroup(const QStringList &elements, const QString &name)
{
    qCDebug(Log) << elements << name;

    // find groups that contain any of the selected elements
    QStringList groupsToUpdate;
    for (const auto &resId : elements) {
        const auto it = m_reservationToGroupMap.constFind(resId);
        if (it == m_reservationToGroupMap.cend()) {
            continue;
        }
        if (!groupsToUpdate.contains(it.value())) {
            groupsToUpdate.push_back(it.value());
        }
    }

    // remove elements from other groups and mark those explicitly grouped
    for (const auto &tgId :groupsToUpdate) {
        auto &tg = m_tripGroups[tgId];
        auto elems = tg.elements();
        for (const auto &resId : elements) {
            elems.removeAll(resId);
        }
        // TODO can elems becomes empty here?
        tg.setElements(elems);
        tg.setIsAutomaticallyGrouped(false);
        if (tg.hasAutomaticName()) {
            tg.setName(guessName(tg.elements()));
        }
        recomputeTripGroupTimes(tg);
        tg.store(fileForGroup(tgId));
        Q_EMIT tripGroupChanged(tgId);
    }

    // create new group
    TripGroup tg;
    tg.setElements(elements);
    tg.setIsAutomaticallyGrouped(false);
    tg.setName(guessName(elements));
    if (tg.name() != name && !name.isEmpty()) {
        tg.setName(name);
        tg.setNameIsAutomatic(false);
    }
    recomputeTripGroupTimes(tg);

    const auto tgId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_tripGroups.insert(tgId, tg);
    for (const auto &resId : elements) {
        m_reservationToGroupMap.insert(resId, tgId);
    }

    tg.store(fileForGroup(tgId));
    Q_EMIT tripGroupAdded(tgId);

    return tgId;
}

#include "moc_tripgroupmanager.cpp"
