/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reservationhelper.h"

#include <KItinerary/BusTrip>
#include <KItinerary/Flight>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <QDateTime>

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
    if (JsonLd::isA<FlightReservation>(lhs)) {
        return lhs.value<FlightReservation>() == rhs.value<FlightReservation>();
    }

    return false;
}

QDateTime ReservationHelper::departureTime(const QVariant &res)
{
    if (JsonLd::isA<TrainReservation>(res)) {
        return res.value<TrainReservation>().reservationFor().value<TrainTrip>().departureTime();
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return res.value<BusReservation>().reservationFor().value<BusTrip>().departureTime();
    }
    if (JsonLd::isA<FlightReservation>(res)) {
        return res.value<FlightReservation>().reservationFor().value<Flight>().departureTime();
    }
    return {};
}

QDateTime ReservationHelper::arrivalTime(const QVariant &res)
{
    if (JsonLd::isA<TrainReservation>(res)) {
        return res.value<TrainReservation>().reservationFor().value<TrainTrip>().arrivalTime();
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return res.value<BusReservation>().reservationFor().value<BusTrip>().arrivalTime();
    }
    if (JsonLd::isA<FlightReservation>(res)) {
        return res.value<FlightReservation>().reservationFor().value<Flight>().arrivalTime();
    }
    return {};
}
