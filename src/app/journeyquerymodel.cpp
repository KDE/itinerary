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

#include <KPublicTransport/Journey>
#include <KPublicTransport/Line>

#include <QDebug>
#include <QUrl>

using namespace KItinerary;

JourneyQueryModel::JourneyQueryModel(QObject *parent)
    : KPublicTransport::JourneyQueryModel(parent)
{
}

JourneyQueryModel::~JourneyQueryModel() = default;

void JourneyQueryModel::setReservationManager(ReservationManager *mgr)
{
    m_resMgr = mgr;
}

void JourneyQueryModel::saveJourney(const QString& batchId, int journeyIndex)
{
    qDebug() << batchId << journeyIndex;
    if (batchId.isEmpty() || journeyIndex < 0 || journeyIndex >= (int)journeys().size()) {
        return;
    }

    const auto journey = journeys().at(journeyIndex);
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
        res = PublicTransport::applyJourneySection(res, sections[0]);
        m_resMgr->updateReservation(resId, res);
//         m_resMgr->addReservation(res);
    }
}
