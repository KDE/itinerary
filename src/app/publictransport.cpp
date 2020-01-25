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

#include "publictransport.h"
#include "logging.h"

#include <KItinerary/BusTrip>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>
#include <KItinerary/Ticket>

#include <KPublicTransport/Attribution>
#include <KPublicTransport/DepartureRequest>
#include <KPublicTransport/Journey>
#include <KPublicTransport/Line>
#include <KPublicTransport/Location>

#include <QDateTime>
#include <QDebug>
#include <QUrl>

static bool isTrainMode(KPublicTransport::Line::Mode mode)
{
    using namespace KPublicTransport;
    switch (mode) {
        case Line::Train:
        case Line::Funicular:
        case Line::LocalTrain:
        case Line::LongDistanceTrain:
        case Line::Metro:
        case Line::RailShuttle:
        case Line::RapidTransit:
        case Line::Tramway:
            return true;
        default:
            return false;
    }
}

static bool isBusMode(KPublicTransport::Line::Mode mode)
{
    using namespace KPublicTransport;
    switch (mode) {
        case Line::Bus:
        case Line::BusRapidTransit:
        case Line::Coach:
            return true;
        default:
            return false;
    }
}

KPublicTransport::Location PublicTransport::locationFromPlace(const QVariant& place, const QVariant &reservation)
{
    using namespace KItinerary;

    KPublicTransport::Location loc;

    if (JsonLd::isA<FlightReservation>(reservation) || JsonLd::isA<TrainReservation>(reservation) || JsonLd::isA<BusReservation>(reservation)) {
        loc.setName(KItinerary::LocationUtil::name(place));
    } else {
        const auto addr = KItinerary::LocationUtil::address(place);
        loc.setStreetAddress(addr.streetAddress());
        loc.setPostalCode(addr.postalCode());
        loc.setLocality(addr.addressLocality());
        loc.setRegion(addr.addressRegion());
        loc.setCountry(addr.addressCountry());
    }

    const auto geo = KItinerary::LocationUtil::geo(place);
    loc.setCoordinate(geo.latitude(), geo.longitude());

    if (KItinerary::JsonLd::canConvert<KItinerary::Place>(place)) {
        const auto p = KItinerary::JsonLd::convert<KItinerary::Place>(place);
        if (!p.identifier().isEmpty()) {
            const auto idSplit = p.identifier().split(QLatin1Char(':'));
            if (idSplit.size() == 2) {
                loc.setIdentifier(idSplit.at(0), idSplit.at(1));
            }
        }
    }

    return loc;
}

QString PublicTransport::lineModeIcon(int lineMode)
{
    using namespace KPublicTransport;
    switch (lineMode) {
        case Line::Air:
            return QStringLiteral("qrc:///images/flight.svg");
        case Line::Boat:
        case Line::Ferry:
            return QStringLiteral("qrc:///images/ferry.svg");
        case Line::Bus:
            return QStringLiteral("qrc:///images/bus.svg");
        case Line::BusRapidTransit:
        case Line::Coach:
            return QStringLiteral("qrc:///images/coach.svg");
        case Line::Funicular:
            return QStringLiteral("qrc:///images/Funicular.svg");
        case Line::LocalTrain:
        case Line::Train:
            return QStringLiteral("qrc:///images/train.svg");
        case Line::LongDistanceTrain:
            return QStringLiteral("qrc:///images/longdistancetrain.svg");
        case Line::Metro:
            return QStringLiteral("qrc:///images/subway.svg");
        case Line::RailShuttle:
        case Line::RapidTransit:
            return QStringLiteral("qrc:///images/rapidtransit.svg");
        case Line::Shuttle:
            return QStringLiteral("qrc:///images/shuttle.svg");
        case Line::Taxi:
            return QStringLiteral("qrc:///images/taxi.svg");
        case Line::Tramway:
            return QStringLiteral("qrc:///images/tramway.svg");
    }

    return QStringLiteral("question");
}

KItinerary::TrainStation PublicTransport::mergeStation(const KItinerary::TrainStation &station, const KPublicTransport::Location &loc)
{
    using namespace KItinerary;
    return MergeUtil::merge(placeFromLocation<TrainStation>(loc), station).value<TrainStation>();
}

static KItinerary::Ticket clearSeat(KItinerary::Ticket ticket)
{
    auto seat = ticket.ticketedSeat();
    seat.setSeatNumber(QString());
    seat.setSeatRow(QString());
    seat.setSeatSection(QString());
    ticket.setTicketedSeat(seat);
    return ticket;
}

static KItinerary::TrainReservation applyJourneySection(KItinerary::TrainReservation res, const KPublicTransport::JourneySection &section)
{
    using namespace KItinerary;

    auto trip = res.reservationFor().value<TrainTrip>();
    trip.setDepartureTime(section.scheduledDepartureTime());
    trip.setArrivalTime(section.scheduledArrivalTime());
    trip.setTrainNumber(section.route().line().name());
    trip.setTrainName(section.route().line().modeString());
    trip.setDeparturePlatform(section.scheduledDeparturePlatform());
    trip.setArrivalPlatform(section.scheduledArrivalPlatform());

    trip.setDepartureStation(PublicTransport::updateToLocation(trip.departureStation(), section.from()));
    trip.setArrivalStation(PublicTransport::updateToLocation(trip.arrivalStation(), section.to()));

    res.setReservationFor(trip);
    res.setReservedTicket(clearSeat(res.reservedTicket().value<Ticket>()));
    return res;
}

static KItinerary::BusReservation applyJourneySection(KItinerary::BusReservation res, const KPublicTransport::JourneySection &section)
{
    using namespace KItinerary;

    auto trip = res.reservationFor().value<BusTrip>();
    trip.setDepartureTime(section.scheduledDepartureTime());
    trip.setArrivalTime(section.scheduledArrivalTime());
    trip.setBusNumber(section.route().line().name());
    trip.setBusName(section.route().line().modeString());
    trip.setDeparturePlatform(section.scheduledDeparturePlatform());
    trip.setArrivalPlatform(section.scheduledArrivalPlatform());

    trip.setDepartureBusStop(PublicTransport::updateToLocation(trip.departureBusStop(), section.from()));
    trip.setArrivalBusStop(PublicTransport::updateToLocation(trip.arrivalBusStop(), section.to()));

    res.setReservationFor(trip);
    res.setReservedTicket(clearSeat(res.reservedTicket().value<Ticket>()));
    return res;
}

QVariant PublicTransport::reservationFromJourneySection(const KPublicTransport::JourneySection &section)
{
    using namespace KItinerary;
    if (isTrainMode(section.route().line().mode())) {
        return ::applyJourneySection(TrainReservation(), section);
    }
    if (isBusMode(section.route().line().mode())) {
        return ::applyJourneySection(BusReservation(), section);
    }

    qCWarning(Log) << "Unsupported section type:" << section.route().line().mode();
    return {};
}

QVariant PublicTransport::applyJourneySection(const QVariant &res, const KPublicTransport::JourneySection &section)
{
    using namespace KItinerary;

    if (JsonLd::isA<TrainReservation>(res)) {
        return ::applyJourneySection(res.value<TrainReservation>(), section);
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return ::applyJourneySection(res.value<BusReservation>(), section);
    }

    qCWarning(Log) << res.typeName() << "Unsupported section type!";
    return res;
}

bool PublicTransport::isSameMode(const QVariant &res, const KPublicTransport::JourneySection &section)
{
    using namespace KPublicTransport;

    if (KItinerary::JsonLd::isA<KItinerary::TrainReservation>(res)) {
        return isTrainMode(section.route().line().mode());
    } else if (KItinerary::JsonLd::isA<KItinerary::BusReservation>(res)) {
        return isBusMode(section.route().line().mode());
    } else if (res.isValid()) {
        qCWarning(Log) << "unexpected reservation type!?" << res;
    }

    return false;
}

QVariant PublicTransport::departureRequestForPlace(const QVariant &place, const QDateTime &dt) const
{
    KPublicTransport::DepartureRequest req;
    req.setDateTime(std::max(dt, QDateTime::currentDateTime()));
    req.setStop(PublicTransport::locationFromPlace(place, {}));
    return QVariant::fromValue(req);
}

QString PublicTransport::attributionSummary(const QVariantList& attributions) const
{
    QStringList l;
    l.reserve(attributions.size());
    for (const auto &v : attributions) {
        const auto attr = v.value<KPublicTransport::Attribution>();
        l.push_back(QLatin1String("<a href=\"") + attr.url().toString() + QLatin1String("\">") + attr.name() + QLatin1String("</a>"));
    }
    return l.join(QLatin1String(", "));
}

bool PublicTransport::warnAboutSection(const KPublicTransport::JourneySection &section) const
{
    using namespace KPublicTransport;

    switch (section.mode()) {
        case JourneySection::Walking:
        case JourneySection::Transfer:
            return section.duration() > 20*60 || section.distance() > 1000;
        case JourneySection::Waiting:
            return section.duration() > 20*60;
        default:
            return false;
    }
}

QString PublicTransport::idenfitierFromLocation(const KPublicTransport::Location &loc)
{
    auto id = loc.identifier(QLatin1String("ibnr"));
    if (!id.isEmpty()) {
        return QLatin1String("ibnr:") + id;
    }
    id = loc.identifier(QLatin1String("uic"));
    if (!id.isEmpty()) {
        return QLatin1String("uic:") + id;
    }
    return {};
}

KItinerary::PostalAddress PublicTransport::addressFromLocation(const KPublicTransport::Location &loc)
{
    KItinerary::PostalAddress addr;
    addr.setStreetAddress(loc.streetAddress());
    addr.setPostalCode(loc.postalCode());
    addr.setAddressLocality(loc.locality());
    addr.setAddressRegion(loc.region());
    addr.setAddressCountry(loc.country());
    return addr;
}

#include "moc_publictransport.cpp"
