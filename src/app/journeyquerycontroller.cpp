/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#include "journeyquerycontroller.h"
#include "logging.h"
#include "reservationmanager.h"
#include "publictransport.h"

#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/Manager>
#include <KPublicTransport/Journey>
#include <KPublicTransport/JourneyReply>
#include <KPublicTransport/JourneyRequest>

#include <QDebug>

using namespace KItinerary;

JourneyQueryController::JourneyQueryController(QObject *parent)
    : QObject(parent)
{
    m_ptMgr.reset(new KPublicTransport::Manager);
}

JourneyQueryController::~JourneyQueryController() = default;

void JourneyQueryController::setReservationManager(ReservationManager *mgr)
{
    m_resMgr = mgr;
}

bool JourneyQueryController::isLoading() const
{
    return m_isLoading;
}

QString JourneyQueryController::errorMessage() const
{
    return m_errorMsg;
}

QVariantList JourneyQueryController::journeys() const
{
    QVariantList l;
    l.reserve(m_journeys.size());
    std::transform(m_journeys.begin(), m_journeys.end(), std::back_inserter(l), [](const auto &journey) { return QVariant::fromValue(journey); });
    return l;
}

void JourneyQueryController::queryJourney(const QString &batchId)
{
    qDebug() << batchId;

    const auto res = m_resMgr->reservation(batchId);
    if (!JsonLd::isA<TrainReservation>(res)) {
        return;
    }

    const auto trip = res.value<TrainReservation>().reservationFor().value<TrainTrip>();

    m_errorMsg.clear();
    emit errorMessageChanged();
    m_isLoading = true;
    emit loadingChanged();

    const auto from = PublicTransport::locationFromStation(trip.departureStation());
    const auto to = PublicTransport::locationFromStation(trip.arrivalStation());
    // TODO consider scheduled time, if in the future
    auto reply = m_ptMgr->queryJourney({from, to});
    QObject::connect(reply, &KPublicTransport::JourneyReply::finished, [reply, this]{
        m_isLoading = false;
        emit loadingChanged();
        if (reply->error() == KPublicTransport::JourneyReply::NoError) {
            m_journeys = reply->takeResult();
            emit journeysChanged();
        } else {
            m_errorMsg = reply->errorString();
            emit errorMessageChanged();
        }
    });
}
