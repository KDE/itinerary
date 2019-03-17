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

#include <KItinerary/BusTrip>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/Manager>
#include <KPublicTransport/Journey>
#include <KPublicTransport/JourneyReply>
#include <KPublicTransport/JourneyRequest>
#include <KPublicTransport/Line>

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
    KPublicTransport::Location from, to;
    if (JsonLd::isA<TrainReservation>(res)) {
        const auto trip = res.value<TrainReservation>().reservationFor().value<TrainTrip>();
        from = PublicTransport::locationFromStation(trip.departureStation());
        to = PublicTransport::locationFromStation(trip.arrivalStation());
    } else if (JsonLd::isA<BusReservation>(res)) {
        const auto trip = res.value<BusReservation>().reservationFor().value<BusTrip>();
        from = PublicTransport::locationFromStation(trip.departureBusStop());
        to = PublicTransport::locationFromStation(trip.arrivalBusStop());
    } else {
        return;
    }

    m_errorMsg.clear();
    emit errorMessageChanged();
    m_isLoading = true;
    emit loadingChanged();

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

static TrainReservation applyJourneySection(TrainReservation res, const KPublicTransport::JourneySection &section)
{
    auto trip = res.reservationFor().value<TrainTrip>();
    trip.setDepartureTime(section.scheduledDepartureTime());
    trip.setArrivalTime(section.scheduledArrivalTime());
    trip.setTrainNumber(section.route().line().name());
    trip.setTrainName(section.route().line().modeString());
    // TODO update to/from locations
    res.setReservationFor(trip);
    return res;
}

static QVariant applyJourneySection(const QVariant &res, const KPublicTransport::JourneySection &section)
{
    // TODO this assumes that both sides are about the same mode of transport (train/bus)
    // this should be ensured below eventually
    if (JsonLd::isA<TrainReservation>(res)) {
        return applyJourneySection(res.value<TrainReservation>(), section);
    }

    qWarning() << "NOT IMPLEMENTED YET!!";
    return res;
}

void JourneyQueryModel::saveJourney(const QString& batchId, int journeyIndex)
{
    qDebug() << batchId << journeyIndex;
    if (batchId.isEmpty() || journeyIndex < 0 || journeyIndex >= (int)m_journeys.size()) {
        return;
    }

    const auto journey = m_journeys.at(journeyIndex);
    std::vector<KPublicTransport::JourneySection> sections;
    std::copy_if(journey.sections().begin(), journey.sections().end(), std::back_inserter(sections), [](const auto &section) {
        return section.mode() == KPublicTransport::JourneySection::PublicTransport;
    });
    if (sections.empty()) {
        return;
    }

    // TODO deal with sections.size() != 1
    if (sections.size() != 1) {
        qWarning() << "NOT IMPLEMENTED YET!!";
        return;
    }

    const auto resIds = m_resMgr->reservationsForBatch(batchId);
    for (const auto &resId : resIds) {
        auto res = m_resMgr->reservation(resId);
        res = applyJourneySection(res, sections[0]);
        m_resMgr->updateReservation(resId, res);
//         m_resMgr->addReservation(res);
    }
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
