/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "gpxexport.h"
#include "favoritelocationmodel.h"
#include "transfer.h"

#include <KItinerary/Event>
#include <KItinerary/LocationUtil>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>

#include <KPublicTransport/Stopover>

#include <KLocalizedString>

using namespace KItinerary;
using namespace KPublicTransport;

GpxExport::GpxExport(QIODevice* out)
    : m_writer(out)
{
    m_writer.writeStartMetadata();
    m_writer.writeLink(QStringLiteral("https://apps.kde.org/itinerary"), i18n("KDE Itinerary"));
    m_writer.writeEndMetadata();
}

GpxExport::~GpxExport() = default;

void GpxExport::writeReservation(const QVariant &res, const KPublicTransport::JourneySection &journey)
{
    if (LocationUtil::isLocationChange(res)) {
        if (!journey.from().isEmpty() && !journey.to().isEmpty()) {
            writeJourneySection(journey);
        } else {
            const auto dep = LocationUtil::departureLocation(res);
            auto coord = LocationUtil::geo(dep);
            m_writer.writeStartRoutePoint(coord.latitude(), coord.longitude());
            m_writer.writeName(LocationUtil::name(dep));
            m_writer.writeTime(SortUtil::startDateTime(res));
            m_writer.writeEndRoutePoint();

            const auto arr = LocationUtil::arrivalLocation(res);
            coord = LocationUtil::geo(arr);
            m_writer.writeStartRoutePoint(coord.latitude(), coord.longitude());
            m_writer.writeName(LocationUtil::name(arr));
            m_writer.writeTime(SortUtil::endDateTime(res));
            m_writer.writeEndRoutePoint();
        }
    } else {
        if (JsonLd::isA<LodgingReservation>(res)) {
            m_writer.writeName(res.value<LodgingReservation>().reservationFor().value<LodgingBusiness>().name());
        } else if (JsonLd::isA<EventReservation>(res)) {
            m_writer.writeName(res.value<EventReservation>().reservationFor().value<Event>().name());
        } else if (JsonLd::isA<FoodEstablishmentReservation>(res)) {
            m_writer.writeName(res.value<FoodEstablishmentReservation>().reservationFor().value<FoodEstablishment>().name());
        }

        const auto loc = LocationUtil::location(res);
        const auto coord = LocationUtil::geo(loc);
        m_writer.writeStartRoutePoint(coord.latitude(), coord.longitude());
        m_writer.writeEndRoutePoint();
        m_writer.writeStartRoutePoint(coord.latitude(), coord.longitude());
        m_writer.writeEndRoutePoint();
    }
}

void GpxExport::writeTransfer(const Transfer &transfer)
{
    const auto journey = transfer.journey();
    for (const auto &section : journey.sections()) {
        writeJourneySection(section);
    }
}

void GpxExport::writeJourneySection(const KPublicTransport::JourneySection &section)
{
    if (section.mode() == JourneySection::Waiting) {
        return;
    }

    if (section.path().isEmpty()) {
        m_writer.writeStartRoutePoint(section.from().latitude(), section.from().longitude());
        m_writer.writeTime(section.hasExpectedDepartureTime() ? section.expectedDepartureTime() : section.scheduledDepartureTime());
        m_writer.writeEndRoutePoint();

        for (const auto &stop : section.intermediateStops()) {
            m_writer.writeStartRoutePoint(stop.stopPoint().latitude(), stop.stopPoint().longitude());
            m_writer.writeName(stop.stopPoint().name());
            m_writer.writeTime(stop.hasExpectedDepartureTime() ? stop.expectedDepartureTime() : stop.scheduledDepartureTime());
            m_writer.writeEndRoutePoint();
        }

        m_writer.writeStartRoutePoint(section.to().latitude(), section.to().longitude());
        m_writer.writeTime(section.hasExpectedArrivalTime() ? section.expectedArrivalTime() : section.scheduledArrivalTime());
        m_writer.writeEndRoutePoint();
    } else {
        for (const auto &pathSec : section.path().sections()) {
            // TODO name?
            for (const auto &pt : pathSec.path()) {
                m_writer.writeStartRoutePoint(pt.y(), pt.x());
                m_writer.writeEndRoutePoint();
            }
        }
    }
}

void GpxExport::writeFavoriteLocation(const FavoriteLocation &fav)
{
    m_writer.writeStartWaypoint(fav.latitude(), fav.longitude());
    m_writer.writeName(fav.name());
    m_writer.writeEndWaypoint();
}
