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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "livedatamanager.h"
#include "logging.h"
#include "pkpassmanager.h"
#include "reservationmanager.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/Departure>
#include <KPublicTransport/DepartureReply>
#include <KPublicTransport/DepartureRequest>
#include <KPublicTransport/Location>
#include <KPublicTransport/Manager>

#include <QDateTime>
#include <QVector>

using namespace KItinerary;

LiveDataManager::LiveDataManager(QObject *parent)
    : QObject(parent)
{
}

LiveDataManager::~LiveDataManager() = default;

void LiveDataManager::setReservationManager(ReservationManager *resMgr)
{
    m_resMgr = resMgr;
    // TODO set up change monitoring

    const auto resIds = resMgr->reservations();
    for (const auto &resId : resIds) {
        const auto res = resMgr->reservation(resId);
        if (!LocationUtil::isLocationChange(res)) { // we only care about transit reservations
            continue;
        }
        if (SortUtil::endtDateTime(res) < QDateTime::currentDateTime().addDays(-1)) {
            continue; // we don't care about past events
        }

        m_reservations.push_back(resId);
    }

    std::sort(m_reservations.begin(), m_reservations.end(), [this](const auto &lhs, const auto &rhs) {
        return SortUtil::isBefore(m_resMgr->reservation(lhs), m_resMgr->reservation(rhs));
    });
}

void LiveDataManager::setPkPassManager(PkPassManager *pkPassMgr)
{
    m_pkPassMgr = pkPassMgr;
}

QVariant LiveDataManager::departure(const QString &resId)
{
    return QVariant::fromValue(m_departures.value(resId));
}

void LiveDataManager::checkForUpdates()
{
    qCDebug(Log) << m_reservations.size();
    m_pkPassMgr->updatePasses(); // TODO do this as part of the below loop

    for (auto it = m_reservations.begin(); it != m_reservations.end();) {
        const auto res = m_resMgr->reservation(*it);

        // clean up old stuff (TODO: do this a bit more precisely)
        if (SortUtil::endtDateTime(res) < QDateTime::currentDateTime().addDays(-1)) {
            it = m_reservations.erase(it);
            continue;
        }

        if (JsonLd::isA<TrainReservation>(res)) {
            checkTrainTrip(res.value<TrainReservation>().reservationFor().value<TrainTrip>(), *it);
        }

        // TODO check for pkpass updates

        ++it;
    }
}

static QString stripSpecial(const QString &str)
{
    QString res;
    res.reserve(str.size());
    std::copy_if(str.begin(), str.end(), std::back_inserter(res), [](const auto c) {
        return c.isLetter() || c.isDigit();
    });
    return res;
}

static bool isSameLine(const QString &lineName, const QString &trainName, const QString &trainNumber)
{
    const auto lhs = stripSpecial(lineName);
    const auto rhs = stripSpecial(trainName + trainNumber);
    return lhs.compare(rhs, Qt::CaseInsensitive) == 0;
}

void LiveDataManager::checkTrainTrip(const TrainTrip& trip, const QString& resId)
{
    qCDebug(Log) << trip.trainName() << trip.trainNumber() << trip.departureTime();
    if (!m_ptMgr) {
        m_ptMgr.reset(new KPublicTransport::Manager);
    }

    using namespace KPublicTransport;

    Location from;
    from.setName(trip.departureStation().name());
    from.setCoordinate(trip.departureStation().geo().latitude(), trip.departureStation().geo().longitude());
    if (!trip.departureStation().identifier().isEmpty()) {
        const auto idSplit = trip.departureStation().identifier().split(QLatin1Char(':'));
        if (idSplit.size() == 2) {
            from.setIdentifier(idSplit.at(0), idSplit.at(1));
        }
    }
    DepartureRequest req(from);
    req.setDateTime(trip.departureTime());

    auto reply = m_ptMgr->queryDeparture(req);
    connect(reply, &Reply::finished, this, [this, trip, resId, reply]() {
        reply->deleteLater();
        if (reply->error() != Reply::NoError) {
            qCDebug(Log) << reply->error() << reply->errorString();
            return;
        }

        for (const auto &dep : reply->departures()) {
            qCDebug(Log) << "Got departure information:" << dep.route().line().name() << dep.scheduledTime() << "for" << trip.trainNumber();
            if (dep.scheduledTime() != trip.departureTime() || !isSameLine(dep.route().line().name(), trip.trainName(), trip.trainNumber())) {
                continue;
            }
            qCDebug(Log) << "Found departure information:" << dep.route().line().name() << dep.expectedPlatform() << dep.expectedTime();
            m_departures.insert(resId, dep);
            emit departureUpdated(resId);
            break;
        }
    });
}

#include "moc_livedatamanager.cpp"
