/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reservationhelper.h"
#include "genericpkpass.h"

#include <KItinerary/BoatTrip>
#include <KItinerary/BusTrip>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/Person>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>
#include <KItinerary/Ticket>

#include <KPublicTransport/Line>
#include <KPublicTransport/RentalVehicle>

#include <QDateTime>

using namespace Qt::Literals::StringLiterals;
using namespace KItinerary;

std::pair<QString, QString> ReservationHelper::lineNameAndNumber(const QVariant &res)
{
    if (JsonLd::isA<TrainReservation>(res)) {
        const auto trip = res.value<TrainReservation>().reservationFor().value<TrainTrip>();
        return std::make_pair(trip.trainName(), trip.trainNumber());
    }
    if (JsonLd::isA<BusReservation>(res)) {
        const auto trip = res.value<BusReservation>().reservationFor().value<BusTrip>();
        return std::make_pair(trip.busName(), trip.busNumber());
    }
    if (JsonLd::isA<FlightReservation>(res)) {
        const auto flight = res.value<FlightReservation>().reservationFor().value<Flight>();
        return std::make_pair(flight.airline().iataCode(), flight.flightNumber());
    }

    return {};
}

bool ReservationHelper::equals(const QVariant &lhs, const QVariant &rhs)
{
    if (lhs.userType() != rhs.userType()) {
        return false;
    }

    if (JsonLd::isA<TrainReservation>(lhs)) {
        return lhs.value<TrainReservation>() == rhs.value<TrainReservation>();
    }
    if (JsonLd::isA<BusReservation>(lhs)) {
        return lhs.value<BusReservation>() == rhs.value<BusReservation>();
    }
    if (JsonLd::isA<BoatReservation>(lhs)) {
        return lhs.value<BoatReservation>() == rhs.value<BoatReservation>();
    }
    if (JsonLd::isA<FlightReservation>(lhs)) {
        return lhs.value<FlightReservation>() == rhs.value<FlightReservation>();
    }

    return false;
}

static QString providerIdentifier(const QVariant &res)
{
    if (JsonLd::isA<TrainReservation>(res)) {
        return res.value<TrainReservation>().reservationFor().value<TrainTrip>().provider().identifier();
    } else if (JsonLd::isA<BusReservation>(res)) {
        return res.value<BusReservation>().reservationFor().value<BusTrip>().provider().identifier();
    }
    return {};
}

QString ReservationHelper::uicCompanyCode(const QVariant &res)
{
    auto id = providerIdentifier(res);
    if (!id.startsWith(QLatin1StringView("uic:")) || id.size() > 8) {
        return {};
    }

    id = id.mid(4);
    while (id.size() < 4) {
        id.insert(0, QLatin1Char('0'));
    }
    return id;
}

QString ReservationHelper::vdvOrganizationId(const QVariant &res)
{
    const auto id = providerIdentifier(res);
    if (!id.startsWith(QLatin1StringView("vdv:"))) {
        return id.mid(4);
    }
    return {};
}

bool ReservationHelper::isUnbound(const QVariant& res)
{
    if (JsonLd::isA<TrainReservation>(res)) {
        return !res.value<TrainReservation>().reservationFor().value<TrainTrip>().departureTime().isValid();
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return !res.value<BusReservation>().reservationFor().value<BusTrip>().departureTime().isValid();
    }
    if (JsonLd::isA<FlightReservation>(res)) {
        return !res.value<FlightReservation>().reservationFor().value<Flight>().departureTime().isValid();
    }
    if (JsonLd::isA<BoatReservation>(res)) {
        return !res.value<BoatReservation>().reservationFor().value<BoatTrip>().departureTime().isValid();
    }
    return false;
}

bool ReservationHelper::isCancelled(const QVariant &res)
{
    return JsonLd::canConvert<Reservation>(res) && JsonLd::convert<Reservation>(res).reservationStatus() == Reservation::ReservationCancelled;
}

QString ReservationHelper::defaultIconName(const QVariant &res)
{
    if (JsonLd::isA<FlightReservation>(res)) {
        return KPublicTransport::Line::modeIconName(KPublicTransport::Line::Air);
    }
    if (JsonLd::isA<TrainReservation>(res)) {
        return KPublicTransport::Line::modeIconName(KPublicTransport::Line::Train);
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return KPublicTransport::Line::modeIconName(KPublicTransport::Line::Bus);
    }
    if (JsonLd::isA<BoatReservation>(res)) {
        return KPublicTransport::Line::modeIconName(KPublicTransport::Line::Ferry);
    }
    if (JsonLd::isA<LodgingReservation>(res)) {
        return u"go-home-symbolic"_s;
    }
    if (JsonLd::isA<EventReservation>(res) || JsonLd::isA<Event>(res)) {
        return u"meeting-attending"_s;
    }
    if (JsonLd::isA<FoodEstablishmentReservation>(res)) {
        return u"qrc:///images/foodestablishment.svg"_s;
    }
    if (JsonLd::isA<RentalCarReservation>(res)) {
        return KPublicTransport::RentalVehicle::vehicleTypeIconName(KPublicTransport::RentalVehicle::Car);
    }

    if (JsonLd::isA<ProgramMembership>(res)) {
        return u"meeting-attending"_s;
    }
    if (JsonLd::isA<Ticket>(res) || JsonLd::isA<GenericPkPass>(res)) {
        return u"bookmarks"_s;
    }

    return {};
}

#include "moc_reservationhelper.cpp"
