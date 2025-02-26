// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "tripgroupmapmodel.h"

#include "publictransport.h"
#include "reservationhelper.h"
#include "reservationmanager.h"
#include "tripgroup.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>

#include <KPublicTransport/Line>

#include <QPolygonF>

using namespace KItinerary;
using namespace KPublicTransport;

// can't follow the palette, we need to match the OSM carto color palette here!'
constexpr inline auto BACKROUND_COLOR = QColor(35, 38, 41);
constexpr inline auto FOREGOUND_COLOR = QColor(239, 240, 241);

TripGroupMapModel::TripGroupMapModel(QObject *parent)
    : QObject(parent)
{
    connect(this, &TripGroupMapModel::tripGroupIdChanged, this, &TripGroupMapModel::recompute);
    connect(this, &TripGroupMapModel::setupChanged, this, &TripGroupMapModel::recompute);
}

TripGroupMapModel::~TripGroupMapModel() = default;

QList<MapPathEntry> TripGroupMapModel::journeySections() const
{
    return m_journeySections;
}

QList<MapPointEntry> TripGroupMapModel::points() const
{
    return m_points;
}

QRectF TripGroupMapModel::boundingBox() const
{
    return m_boundingBox;
}

void TripGroupMapModel::recompute()
{
    if (!m_tripGroupMgr || m_tripGroupId.isEmpty() || !m_liveDataMgr || !m_transferMgr) {
        return;
    }

    m_journeySections.clear();
    m_points.clear();
    m_boundingBox = {};

    const auto elems = m_tripGroupMgr->tripGroup(m_tripGroupId).elements();
    for (const auto &resId : elems) {
        // pre transfer
        if (const auto transfer = m_transferMgr->transfer(resId, Transfer::Before); transfer.state() == Transfer::Selected) {
            expandJourney(transfer.journey());
        }

        // reservation
        const auto res = m_tripGroupMgr->reservationManager()->reservation(resId);
        if (LocationUtil::isLocationChange(res)) {
            const auto jny = m_liveDataMgr->journey(resId);
            if (jny.mode() != JourneySection::Invalid) {
                expandJourneySection(jny);
            } else {
                KPublicTransport::JourneySection jnySec;
                jnySec.setFrom(PublicTransport::locationFromPlace(LocationUtil::departureLocation(res), res));
                jnySec.setTo(PublicTransport::locationFromPlace(LocationUtil::arrivalLocation(res), res));
                jnySec.setScheduledDepartureTime(SortUtil::startDateTime(res));
                jnySec.setScheduledArrivalTime(SortUtil::endDateTime(res));
                jnySec.setMode(JourneySection::PublicTransport);
                if (JsonLd::isA<FlightReservation>(res)) {
                    Line line;
                    line.setMode(Line::Air);
                    Route route;
                    route.setLine(line);
                    jnySec.setRoute(route);
                }
                expandJourneySection(jnySec);
            }
        } else {
            MapPointEntry point;
            point.location = PublicTransport::locationFromPlace(LocationUtil::location(res), res);
            if (!point.location.hasCoordinate()) {
                continue;
            }
            point.color = BACKROUND_COLOR;
            point.textColor = FOREGOUND_COLOR;
            point.iconName = ReservationHelper::defaultIconName(res);
            m_boundingBox |= QRectF(QPointF(point.location.longitude() - 0.001, point.location.latitude() - 0.001),
                                    QPointF(point.location.longitude() + 0.001, point.location.latitude() + 0.001));
            m_points.push_back(std::move(point));
        }

        // post transfer
        if (const auto transfer = m_transferMgr->transfer(resId, Transfer::After); transfer.state() == Transfer::Selected) {
            expandJourney(transfer.journey());
        }
    }

    // merge adjacent walking sections
    for (qsizetype i = 1; i < m_journeySections.size(); ++i) {
        if (m_journeySections[i-1].journeySection.mode() != JourneySection::Walking || m_journeySections[i].journeySection.mode() != JourneySection::Walking) {
            continue;
        }
        if (Location::distance(m_journeySections[i-1].journeySection.to(), m_journeySections[i].journeySection.from()) > 50) {
            continue;
        }
        if (m_journeySections[i-1].journeySection.scheduledArrivalTime().secsTo(m_journeySections[i].journeySection.scheduledDepartureTime()) > 3600) {
            continue;
        }
        auto path = m_journeySections[i-1].journeySection.path();
        auto pathSecs = path.takeSections();
        auto pathSecs2 = m_journeySections[i].journeySection.path().takeSections();
        std::move(pathSecs2.begin(), pathSecs2.end(), std::back_inserter(pathSecs));
        path.setSections(std::move(pathSecs));
        m_journeySections[i-1].journeySection.setPath(path);
        m_journeySections[i-1].journeySection.setScheduledArrivalTime(m_journeySections[i].journeySection.scheduledArrivalTime());

        m_journeySections.remove(i, 1);
        --i;
    }

    // improve overlapping stop points:
    // prefer those tied to a public transport section, as those have time/platform information
    for (qsizetype i = 0; i < m_journeySections.size(); ++i) {
        if (m_journeySections[i].journeySection.mode() != JourneySection::PublicTransport) {
            continue;
        }
        if (i > 0 && m_journeySections[i-1].journeySection.mode() != JourneySection::PublicTransport) {
            m_journeySections[i-1].showEnd = Location::distance(m_journeySections[i-1].journeySection.to(), m_journeySections[i].journeySection.from()) > 5.0;
        }
        if (i < m_journeySections.size() - 1 && m_journeySections[i+1].journeySection.mode() != JourneySection::PublicTransport) {
            m_journeySections[i+1].showStart = Location::distance(m_journeySections[i].journeySection.to(), m_journeySections[i+1].journeySection.from()) > 5.0;
        }
    }

    Q_EMIT contentChanged();
}

void TripGroupMapModel::expandJourney(const KPublicTransport::Journey &jny)
{
    for (const auto &jnySec : jny.sections()) {
        expandJourneySection(jnySec);
    }
}

void TripGroupMapModel::expandJourneySection(const KPublicTransport::JourneySection &jnySec)
{
    if (!jnySec.from().hasCoordinate() || !jnySec.to().hasCoordinate()) {
        return;
    }

    MapPathEntry entry;
    entry.journeySection = jnySec;

    entry.color = BACKROUND_COLOR;
    entry.textColor = FOREGOUND_COLOR;

    switch (jnySec.mode()) {
    case JourneySection::Invalid:
    case JourneySection::Waiting:
        return;
    case JourneySection::PublicTransport:
        if (jnySec.route().line().hasColor()) {
            entry.color = jnySec.route().line().color();
        }
        if (jnySec.route().line().hasTextColor()) {
            entry.textColor = jnySec.route().line().textColor();
        }
        entry.width = jnySec.route().line().mode() == Line::Mode::Air ? 2.0 : 10.0;
        break;
    case JourneySection::Transfer:
        entry.journeySection.setMode(JourneySection::Walking); // simplifies checks down the line
        [[fallthrough]];
    case JourneySection::Walking:
    case JourneySection::IndividualTransport:
    case JourneySection::RentedVehicle:
        entry.width = 4.0;
        break;
    }

    // generate path if missing
    auto jnyPath = jnySec.path();
    if (jnyPath.sections().empty()) {
        PathSection pathSec;
        pathSec.setPath({QPointF{jnySec.from().longitude(), jnySec.from().latitude()}, QPointF{jnySec.to().longitude(), jnySec.to().latitude()}});
        jnyPath.setSections({pathSec});
    }

    for (const auto &pathSec : jnyPath.sections()) {
        m_boundingBox |= pathSec.path().boundingRect();
    }

    m_journeySections.push_back(std::move(entry));
}

#include "moc_tripgroupmapmodel.cpp"
