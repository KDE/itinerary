/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "tripgroupmanager.h"
#include "tripgroup.h"
#include "logging.h"
#include "reservationmanager.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/MergeUtil>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>

#include <QDateTime>
#include <QDebug>
#include <QDirIterator>
#include <QStandardPaths>
#include <QUuid>

using namespace KItinerary;

enum {
    MaximumTripDuration = 20, // in days
    MaximumTripElements = 20
};

TripGroupManager::TripGroupManager(QObject* parent) :
    QObject(parent)
{
    load();
}

TripGroupManager::~TripGroupManager() = default;

void TripGroupManager::setReservationManager(ReservationManager *resMgr)
{
    m_resMgr = resMgr;
    connect(m_resMgr, &ReservationManager::reservationAdded, this, &TripGroupManager::reservationAdded);
    connect(m_resMgr, &ReservationManager::reservationUpdated, this, &TripGroupManager::reservationChanged);
    connect(m_resMgr, &ReservationManager::reservationRemoved, this, &TripGroupManager::reservationRemoved);

    const auto allReservations = m_resMgr->reservations();
    m_reservations.clear();
    m_reservations.reserve(allReservations.size());
    std::copy(allReservations.begin(), allReservations.end(), std::back_inserter(m_reservations));
    std::sort(m_reservations.begin(), m_reservations.end(), [this](const auto &lhs, const auto &rhs) {
        return SortUtil::isBefore(m_resMgr->reservation(lhs), m_resMgr->reservation(rhs));
    });

    scanAll();
}

TripGroup TripGroupManager::tripGroup(const QString &id) const
{
    return m_tripGroups.value(id);
}

TripGroup TripGroupManager::tripGroupForReservation(const QString &resId) const
{
    return tripGroup(m_reservationToGroupMap.value(resId));
}

QString TripGroupManager::basePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/tripgroups/");
}

void TripGroupManager::load()
{
    const auto base = basePath();
    QDir::root().mkpath(base);

    for (QDirIterator it(base, QDir::NoDotAndDotDot | QDir::Files); it.hasNext();) {
        it.next();
        TripGroup g(this);
        if (g.load(it.fileName())) {
            const auto tgId = it.fileInfo().baseName();
            m_tripGroups.insert(tgId, g);
            for (const auto &resId : g.elements()) {
                m_reservationToGroupMap.insert(resId, tgId);
            }
        }
    }
}

void TripGroupManager::clear()
{
    qCDebug(Log) << "deleting" << basePath();
    QDir d(basePath());
    d.removeRecursively();
}

void TripGroupManager::reservationAdded(const QString &resId)
{
    // TODO
}

void TripGroupManager::reservationChanged(const QString &resId)
{
    // ### we can probably make this more efficient
    reservationRemoved(resId);
    reservationAdded(resId);
}

void TripGroupManager::reservationRemoved(const QString &resId)
{
    // TODO
}

void TripGroupManager::scanAll()
{
    qCDebug(Log);
    for (auto it = m_reservations.begin(); it != m_reservations.end(); ++it) {
        if (m_reservationToGroupMap.contains(*it)) { // already in a group
            continue;
        }

        // not a location change? -> continue
        if (!LocationUtil::isLocationChange(m_resMgr->reservation(*it))) {
            continue;
        }

        scanOne(it);
    }
}

void TripGroupManager::scanOne(const std::vector<QString>::const_iterator &beginIt)
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

    // scan by location change
    for (auto it = beginIt + 1; it != m_reservations.end(); ++it) {
        if (m_reservationToGroupMap.contains(*it)) {
            break; // TODO do we need to check if we can merge with this?
        }

        const auto prevRes = res;
        const auto curRes = m_resMgr->reservation(*it);

        // not a location change? -> continue searching
        if (!LocationUtil::isLocationChange(curRes)) {
            continue;
        }
        res = curRes;

        // is this a multi-traveler element? -> continue searching
        Q_ASSERT(JsonLd::canConvert<Reservation>(prevRes));
        Q_ASSERT(JsonLd::canConvert<Reservation>(res));
        const auto prevTrip = JsonLd::convert<Reservation>(prevRes).reservationFor();
        const auto curTrip = JsonLd::convert<Reservation>(res).reservationFor();
        if (MergeUtil::isSame(prevTrip, curTrip)) {
            // if the result iterators are on it -1 we should advanced them here too
            if (connectedIt == it - 1) {
                ++connectedIt;
            }
            if (resNumIt == it - 1) {
                ++resNumIt;
            }
            continue;
        }

        // all search strategies think they are done
        if (resNumSearchDone && connectedSearchDone) {
            break;
        }

        // search depth reached
        // ### we probably don't want to count multi-traveler elements for this!
        if (std::distance(beginIt, it) > MaximumTripElements) {
            qDebug() << "aborting search, maximum search depth reached";
            break;
        }

        // maximum trip duration exceeded?
        const auto endDt = SortUtil::endtDateTime(res);
        if (beginDt.daysTo(endDt) > MaximumTripDuration) {
            qDebug() << "aborting search, maximum trip duration reached";
            break;
        }

        if (!connectedSearchDone) {
            // not an adjacent location change? -> not a trip group
            const auto prevArrival = LocationUtil::arrivalLocation(prevRes);
            const auto curDeparture = LocationUtil::departureLocation(res);
            qDebug() << "  changing from" << LocationUtil::name(prevArrival) << "to" << LocationUtil::name(curDeparture);
            if (!LocationUtil::isSameLocation(prevArrival, curDeparture, LocationUtil::CityLevel)) {
                qDebug() << "aborting connectivity search, not an adjacent transition from" << LocationUtil::name(prevArrival) << "to" << LocationUtil::name(curDeparture);
                connectedIt = m_reservations.end();
                connectedSearchDone = true;
            } else {
                connectedIt = it;
            }

            // same location as beginIt? -> we reached the end of the trip (break)
            const auto curArrival = LocationUtil::arrivalLocation(res);
            qDebug() << prevRes << res << "  current transition goes to" << LocationUtil::name(curArrival);
            if (LocationUtil::isSameLocation(beginDeparture, curArrival, LocationUtil::CityLevel)) {
                connectedSearchDone = true;
            }
        }

        if (!resNumSearchDone &&  JsonLd::canConvert<Reservation>(res)) {
            const auto resNum = JsonLd::convert<Reservation>(res).reservationNumber();
            const auto r = std::find_if(m_resNumSearch.begin(), m_resNumSearch.end(), [res](const auto &elem) {
                return elem.type == res.userType();
            });
            if (r == m_resNumSearch.end()) {
                // mode of transport changed: we consider this still part of the trip if connectivity
                // search thinks this is part of the same trip too
                if (!connectedSearchDone) {
                    m_resNumSearch.push_back({beginRes.userType(), resNum});
                }
            } else {
                if (!resNum.isEmpty() && (*r).resNum == resNum) {
                    resNumIt = it;
                } else {
                    qDebug() << "aborting reservation number search due to mismatch";
                    resNumSearchDone = true;
                }
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

    if (it == m_reservations.end() || std::distance(beginIt, it) < 2) {
        return;
    }
    ++it; // so this marks the end

    // create a trip for [beginIt, it)
    qDebug() << "creating trip group" << LocationUtil::name(beginDeparture) << LocationUtil::name(LocationUtil::arrivalLocation(res));
    const auto tgId = QUuid::createUuid().toString();
    TripGroup g(this);
    // TODO determine name
    QVector<QString> elems;
    elems.reserve(std::distance(beginIt, it));
    std::copy(beginIt, it, std::back_inserter(elems));
    g.setElements(elems);
    for (auto it2 = beginIt; it2 != it; ++it2) {
        m_reservationToGroupMap.insert(*it2, tgId);
    }
    m_tripGroups.insert(tgId, g);
    g.store(basePath() + tgId + QLatin1String(".json"));
    emit tripGroupAdded(tgId);
}
