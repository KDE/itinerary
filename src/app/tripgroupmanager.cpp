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
constexpr inline const auto MaximumTripElements = 20;
constexpr inline const auto MinimumTripElements = 2;

TripGroupManager::TripGroupManager(QObject* parent) :
    QObject(parent)
{
    load();
}

TripGroupManager::~TripGroupManager() = default;

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

QList<QString> TripGroupManager::tripGroups() const
{
    QList<QString> groups;
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

void TripGroupManager::load()
{
    const auto base = basePath();
    QDir::root().mkpath(base);

    for (QDirIterator it(base, QDir::NoDotAndDotDot | QDir::Files); it.hasNext();) {
        it.next();
        TripGroup g(this);
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
    if (!QFile::remove(basePath() + groupId + ".json"_L1)) {
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

void TripGroupManager::removeReservationsInGroup(const QString &groupId)
{
    const auto groupIt = m_tripGroups.constFind(groupId);
    if (groupIt == m_tripGroups.constEnd()) {
        return;
    }

    const auto elements = groupIt.value().elements();
    for (const auto &element : elements) {
        m_resMgr->removeBatch(element);
    }
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
    // ### we can probably make this more efficient
    batchRemoved(resId);
    batchAdded(resId);
}

void TripGroupManager::batchRenamed(const QString &oldBatchId, const QString &newBatchId)
{
    // ### this can be done more efficiently
    batchRemoved(oldBatchId);
    batchAdded(newBatchId);
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
        if (elems.size() < MinimumTripElements) { // group deleted
            qDebug() << "removing trip group due to getting too small";
            removeTripGroup(groupId);
        } else { // group changed
            qDebug() << "removing element from trip group" << resId << elems;
            groupIt.value().setElements(elems);
            recomputeTripGroupTimes(groupIt.value());
            groupIt.value().store(basePath() + mapIt.value() + ".json"_L1);
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
            tgIt.value().store(basePath() + tgIt.key() + ".json"_L1);
            Q_EMIT tripGroupChanged(tgId);
        }
    }
}

void TripGroupManager::scanAll()
{
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

        // not a location change? -> continue
        if (!LocationUtil::isLocationChange(m_resMgr->reservation(*it))) {
            continue;
        }

        scanOne(it);
        prevGroup = m_reservationToGroupMap.value(*it);
    }
}

static bool isConnectedTransition(const QVariant &fromRes, const QVariant &toRes)
{
    const auto from = LocationUtil::arrivalLocation(fromRes);
    const auto to = LocationUtil::departureLocation(toRes);
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

    qDebug() << "starting scan at" << LocationUtil::name(beginDeparture);
    auto res = beginRes;
    auto resNumIt = m_reservations.cend(); // result of the search using reservation ids
    auto connectedIt = m_reservations.cend(); // result of the search using trip connectivity

    bool resNumSearchDone = false;
    bool connectedSearchDone = false;
    bool reachedStartAgain = false;

    // scan by location change
    for (auto it = beginIt + 1; it != m_reservations.end(); ++it) {
        const auto prevRes = res;
        const auto curRes = m_resMgr->reservation(*it);

        // not a location change? -> continue searching
        if (!LocationUtil::isLocationChange(curRes)) {
            continue;
        }
        res = curRes;

        // all search strategies think they are done
        if (resNumSearchDone && connectedSearchDone) {
            break;
        }

        // search depth reached
        // ### we probably don't want to count multi-traveler elements for this!
        if (std::distance(beginIt, it) > MaximumTripElements) {
            qDebug() << "  aborting search, maximum search depth reached";
            break;
        }

        // maximum trip duration exceeded?
        const auto endDt = SortUtil::endDateTime(res);
        if (beginDt.daysTo(endDt) > MaximumTripDuration) {
            qDebug() << "  aborting search, maximum trip duration reached";
            break;
        }

        // check for connected transitions (ie. previous arrival == current departure)
        const auto prevArrival = LocationUtil::arrivalLocation(prevRes);
        const auto curDeparture = LocationUtil::departureLocation(res);
        const auto connectedTransition = isConnectedTransition(prevRes, res);
        qDebug() << "  current transition goes from" << LocationUtil::name(prevArrival) << "to" << LocationUtil::name(LocationUtil::arrivalLocation(res));

        if (!connectedSearchDone) {
            if (!connectedTransition) {
                qDebug() << "  aborting connectivity search, not an adjacent transition from" << LocationUtil::name(prevArrival) << "to" << LocationUtil::name(curDeparture);
                connectedIt = m_reservations.end();
                connectedSearchDone = true;
            } else {
                connectedIt = it;
            }

            // same location as beginIt? -> we reached the end of the trip (break)
            const auto curArrival = LocationUtil::arrivalLocation(res);
            if (LocationUtil::isSameLocation(beginDeparture, curArrival, LocationUtil::CityLevel)) {
                qDebug() << "  aborting connectivity search, arrived at the start again" << LocationUtil::name(curArrival);
                connectedSearchDone = true;
                reachedStartAgain = true;
            }
        }

        if (!resNumSearchDone &&  JsonLd::canConvert<Reservation>(res)) {
            const auto resNum = JsonLd::convert<Reservation>(res).reservationNumber();
            if (!resNum.isEmpty()) {
                const auto r = std::find_if(m_resNumSearch.begin(), m_resNumSearch.end(), [res, resNum](const auto &elem) {
                    return elem.type == res.userType() && elem.resNum == resNum;
                });
                if (r == m_resNumSearch.end()) {
                    // mode of transport or reservation changed: we consider this still part of the trip if connectivity
                    // search thinks this is part of the same trip too, and we are not at home again yet
                    if (connectedTransition && !LocationUtil::isSameLocation(prevArrival, beginDeparture, LocationUtil::CityLevel)) {
                        qDebug() << "  considering transition to" << LocationUtil::name(LocationUtil::arrivalLocation(res)) << "as part of trip despite unknown reservation number";
                        m_resNumSearch.push_back({res.userType(), resNum});
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
    }

    // determine which search strategy found the larger result
    auto it = m_reservations.cend();
    if (!connectedSearchDone) {
        connectedIt = m_reservations.end();
    }
    if (connectedIt != m_reservations.end() && resNumIt != m_reservations.end()) {
        it = std::max(connectedIt, resNumIt);
    } else {
        it = connectedIt == m_reservations.end() ? resNumIt : connectedIt;
    }

    if (it == m_reservations.end()) {
        qDebug() << "nothing found";
        return;
    }

    // remove leading loop appendices (trailing ones will be cut by the loop check above already)
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
            }
            beginIt = it2;
            break;
        }
    }

    if (std::distance(beginIt, it) < MinimumTripElements - 1) {
        qDebug() << "trip too short";
        return;
    }

    // create a trip for [beginIt, it)
    ++it; // so this marks the end
    QList<QString> elems;
    elems.reserve(std::distance(beginIt, it));
    std::copy(beginIt, it, std::back_inserter(elems));

    // if we are looking at an existing group, did that expand?
    const auto groupIt = m_tripGroups.find(m_reservationToGroupMap.value(*beginIt));
    if (groupIt != m_tripGroups.end() && groupIt.value().elements() == elems) {
        qDebug() << "existing group unchanged" << groupIt.value().name();
        return;
    }

    std::set<QString> pendingGroupRemovals;
    if (groupIt == m_tripGroups.end()) {
        const auto tgId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        TripGroup g(this);
        g.setElements(elems);
        for (auto it2 = beginIt; it2 != it; ++it2) {
            // remove overlapping/nested groups, delay this until the end though, as that will invalidate our iterators
            const auto previousGroupId = m_reservationToGroupMap.value(*it2);
            if (!previousGroupId.isEmpty() && previousGroupId != tgId) {
                pendingGroupRemovals.insert(previousGroupId);
            }
            m_reservationToGroupMap.insert(*it2, tgId);
        }
        g.setName(guessName(g));
        recomputeTripGroupTimes(g);
        qDebug() << "creating trip group" << g.name();
        m_tripGroups.insert(tgId, g);
        g.store(basePath() + tgId + ".json"_L1);
        Q_EMIT tripGroupAdded(tgId);
    } else {
        auto &g = groupIt.value();
        for (const auto &elem : g.elements()) { // remove old element mappings, some of them might no longer be valid
            m_reservationToGroupMap.remove(elem);
        }
        g.setElements(elems);
        for (auto it2 = beginIt; it2 != it; ++it2) {
            m_reservationToGroupMap.insert(*it2, groupIt.key());
        }
        g.setName(guessName(g));
        recomputeTripGroupTimes(g);
        qDebug() << "updating trip group" << g.name();
        g.store(basePath() + groupIt.key() + ".json"_L1);
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
        it.value().store(basePath() + it.key() + ".json"_L1);
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

QString TripGroupManager::guessDestinationFromLodging(const TripGroup &g) const
{
    // we assume that lodging indicates the actual destination, not a stopover location
    QStringList dests;
    for (const auto &resId : g.elements()) {
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

bool TripGroupManager::isRoundTrip(const TripGroup& g) const
{
    const auto depId = g.elements().at(0);
    const auto arrId = g.elements().constLast();
    const auto dep = LocationUtil::departureLocation(m_resMgr->reservation(depId));
    const auto arr = LocationUtil::arrivalLocation(m_resMgr->reservation(arrId));
    return LocationUtil::isSameLocation(dep, arr, LocationUtil::CityLevel);
}

QString TripGroupManager::guessDestinationFromTransportTimeGap(const TripGroup &g) const
{
    // we must only do this for return trips
    if (!isRoundTrip(g)) {
        return {};
    }

    // we assume that the largest time interval between arrival and departure of two adjacent location changes is the destination
    QDateTime beginDt;
    QString destName;
    qint64 maxLength = 0;

    for (const auto &resId : g.elements()) {
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

QString TripGroupManager::guessName(const TripGroup& g) const
{
    // part 1: the destination of the trip
    QString dest = guessDestinationFromLodging(g);
    if (dest.isEmpty()) {
        dest = guessDestinationFromTransportTimeGap(g);
    }
    if (dest.isEmpty()) {
        // two fallback cases: round-trips and one-way trips
        const auto beginLoc = LocationUtil::departureLocation(m_resMgr->reservation(g.elements().at(0)));
        const auto endLoc = LocationUtil::arrivalLocation(m_resMgr->reservation(g.elements().constLast()));
        if (LocationUtil::isSameLocation(beginLoc, endLoc, LocationUtil::CityLevel)) {
            const auto middleIdx = (g.elements().size() - 1 + (g.elements().size() % 2)) / 2;
            const auto middleRes = m_resMgr->reservation(g.elements().at(middleIdx));
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

    // part 2: the time range of the trip
    // three cases: within 1 month, crossing a month boundary in one year, crossing a year boundary
    const auto beginDt = SortUtil::startDateTime(m_resMgr->reservation(g.elements().at(0)));
    const auto endDt = SortUtil::endDateTime(m_resMgr->reservation(g.elements().constLast()));
    Q_ASSERT(beginDt.daysTo(endDt) <= MaximumTripDuration);
    if (beginDt.date().year() == endDt.date().year()) {
        if (beginDt.date().month() == endDt.date().month()) {
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
    }
    auto change = dt != tg.beginDateTime();
    tg.setBeginDateTime(dt);

    res = m_resMgr->reservation(tg.elements().constLast());
    dt = KItinerary::SortUtil::endDateTime(res);

    transfer = m_transferMgr->transfer(tg.elements().constLast(), Transfer::After);
    if (transfer.state() == Transfer::Selected && transfer.journey().scheduledArrivalTime().isValid()) {
        dt = std::max(dt, transfer.journey().scheduledArrivalTime());
    }
    change |= dt != tg.endDateTime();
    tg.setEndDateTime(dt);
    return change;
}

#include "moc_tripgroupmanager.cpp"
