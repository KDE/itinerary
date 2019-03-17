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

#include "journeyquerymodel.h"
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

JourneyQueryModel::JourneyQueryModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

JourneyQueryModel::~JourneyQueryModel() = default;

void JourneyQueryModel::setReservationManager(ReservationManager *mgr)
{
    m_resMgr = mgr;
}

void JourneyQueryModel::setPublicTransportManager(KPublicTransport::Manager *mgr)
{
    m_ptMgr = mgr;
}

bool JourneyQueryModel::isLoading() const
{
    return m_isLoading;
}

QString JourneyQueryModel::errorMessage() const
{
    return m_errorMsg;
}

void JourneyQueryModel::queryJourney(const QString &batchId)
{
    beginResetModel();
    m_journeys.clear();
    endResetModel();

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
    QObject::connect(reply, &KPublicTransport::JourneyReply::finished, this, [reply, this]{
        m_isLoading = false;
        emit loadingChanged();
        if (reply->error() == KPublicTransport::JourneyReply::NoError) {
            beginResetModel();
            m_journeys = reply->takeResult();
            endResetModel();
        } else {
            m_errorMsg = reply->errorString();
            emit errorMessageChanged();
        }
        reply->deleteLater();
    });
}

int JourneyQueryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_journeys.size();
}

QVariant JourneyQueryModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
        case JourneyRole:
            return QVariant::fromValue(m_journeys[index.row()]);
    }
    return {};
}

QHash<int, QByteArray> JourneyQueryModel::roleNames() const
{
    auto r = QAbstractListModel::roleNames();
    r.insert(JourneyRole, "journey");
    return r;
}
