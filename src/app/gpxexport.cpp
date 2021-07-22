/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "gpxexport.h"
#include "favoritelocationmodel.h"
#include "transfer.h"

#include <KItinerary/BusTrip>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/LocationUtil>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>
#include <KItinerary/TrainTrip>
#include <KItinerary/Visit>

#include <KPublicTransport/Journey>
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

void GpxExport::writeReservation(const QVariant &res, const KPublicTransport::JourneySection &journey, const Transfer &before, const Transfer &after)
{
    if (LocationUtil::isLocationChange(res)) {
        m_writer.writeStartRoute();
        const auto dep = LocationUtil::departureLocation(res);
        const auto arr = LocationUtil::arrivalLocation(res);

        if (JsonLd::isA<FlightReservation>(res)) {
            m_writer.writeName(i18n("Flight %1 from %2 to %3", res.value<FlightReservation>().reservationFor().value<Flight>().flightNumber(), LocationUtil::name(dep), LocationUtil::name(arr)));
        } else if (JsonLd::isA<TrainReservation>(res)) {
            const auto train = res.value<TrainReservation>().reservationFor().value<TrainTrip>();
            const QString n = train.trainName() + QLatin1Char(' ') + train.trainNumber();
            m_writer.writeName(i18n("Train %1 from %2 to %3", n.trimmed(), LocationUtil::name(dep), LocationUtil::name(arr)));
        } else if (JsonLd::isA<BusReservation>(res)) {
            const auto bus = res.value<BusReservation>().reservationFor().value<BusTrip>();
            const QString n = bus.busName() + QLatin1Char(' ') + bus.busNumber();
            m_writer.writeName(i18n("Bus %1 from %2 to %3", n.trimmed(), LocationUtil::name(dep), LocationUtil::name(arr)));
        }

        writeTransfer(before);
        if (!journey.from().isEmpty() && !journey.to().isEmpty()) {
            writeJourneySection(journey);
        } else {
            auto coord = LocationUtil::geo(dep);
            if (coord.isValid()) {
                m_writer.writeStartRoutePoint(coord.latitude(), coord.longitude());
                m_writer.writeName(LocationUtil::name(dep));
                m_writer.writeTime(SortUtil::startDateTime(res));
                m_writer.writeEndRoutePoint();
            }

            coord = LocationUtil::geo(arr);
            if (coord.isValid()) {
                m_writer.writeStartRoutePoint(coord.latitude(), coord.longitude());
                m_writer.writeName(LocationUtil::name(arr));
                m_writer.writeTime(SortUtil::endDateTime(res));
                m_writer.writeEndRoutePoint();
            }
        }
        writeTransfer(after);
        m_writer.writeEndRoute();

        // waypoint for departure
        auto coord = LocationUtil::geo(dep);
        if (coord.isValid()) {
            m_writer.writeStartWaypoint(coord.latitude(), coord.longitude());
            m_writer.writeName(LocationUtil::name(dep));
            m_writer.writeTime(SortUtil::startDateTime(res));
            m_writer.writeEndWaypoint();
        }

        // waypoints for intermediate stops
        for (const auto &stop : journey.intermediateStops()) {
            if (!stop.stopPoint().hasCoordinate()) {
                continue;
            }
            m_writer.writeStartWaypoint(stop.stopPoint().latitude(), stop.stopPoint().longitude());
            m_writer.writeName(stop.stopPoint().name());
            m_writer.writeTime(stop.hasExpectedDepartureTime() ? stop.expectedDepartureTime() : stop.scheduledDepartureTime());
            m_writer.writeEndWaypoint();
        }

        // waypoint for arrival
        coord = LocationUtil::geo(arr);
        if (coord.isValid()) {
            m_writer.writeStartWaypoint(coord.latitude(), coord.longitude());
            m_writer.writeName(LocationUtil::name(arr));
            m_writer.writeTime(SortUtil::endDateTime(res));
            m_writer.writeEndWaypoint();
        }

    } else {
        writeSelfContainedTransfer(before);
        const auto loc = LocationUtil::location(res);
        const auto coord = LocationUtil::geo(loc);
        if (coord.isValid()) {
            m_writer.writeStartWaypoint(coord.latitude(), coord.longitude());
            if (JsonLd::isA<LodgingReservation>(res)) {
                m_writer.writeName(res.value<LodgingReservation>().reservationFor().value<LodgingBusiness>().name());
            } else if (JsonLd::isA<EventReservation>(res)) {
                m_writer.writeName(res.value<EventReservation>().reservationFor().value<Event>().name());
            } else if (JsonLd::isA<FoodEstablishmentReservation>(res)) {
                m_writer.writeName(res.value<FoodEstablishmentReservation>().reservationFor().value<FoodEstablishment>().name());
            } else if (JsonLd::isA<TouristAttractionVisit>(res)) {
                m_writer.writeName(res.value<TouristAttractionVisit>().touristAttraction().name());
            }
            m_writer.writeTime(SortUtil::startDateTime(res));
            m_writer.writeEndWaypoint();
        }
        writeSelfContainedTransfer(after);
    }
}

void GpxExport::writeSelfContainedTransfer(const Transfer& transfer)
{
    if (transfer.state() != Transfer::Selected) {
        return;
    }

    m_writer.writeStartRoute();
    // TODO name
    writeTransfer(transfer);
    m_writer.writeEndRoute();
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
        m_writer.writeName(section.from().name());
        m_writer.writeTime(section.hasExpectedDepartureTime() ? section.expectedDepartureTime() : section.scheduledDepartureTime());
        m_writer.writeEndRoutePoint();

        for (const auto &stop : section.intermediateStops()) {
            m_writer.writeStartRoutePoint(stop.stopPoint().latitude(), stop.stopPoint().longitude());
            m_writer.writeName(stop.stopPoint().name());
            m_writer.writeTime(stop.hasExpectedDepartureTime() ? stop.expectedDepartureTime() : stop.scheduledDepartureTime());
            m_writer.writeEndRoutePoint();
        }

        m_writer.writeStartRoutePoint(section.to().latitude(), section.to().longitude());
        m_writer.writeName(section.to().name());
        m_writer.writeTime(section.hasExpectedArrivalTime() ? section.expectedArrivalTime() : section.scheduledArrivalTime());
        m_writer.writeEndRoutePoint();
    } else {
        for (const auto &pathSec : section.path().sections()) {
            // TODO name/time
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
