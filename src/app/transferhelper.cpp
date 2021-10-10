/*
    SPDX-FileCopyrightText: 2020-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "transferhelper.h"

#include <KItinerary/BusTrip>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <QDateTime>
#include <QVariant>

using namespace KItinerary;

QDateTime TransferHelper::anchorTimeBefore(const QVariant &res)
{
    if (JsonLd::isA<TrainReservation>(res)) {
        return res.value<TrainReservation>().reservationFor().value<TrainTrip>().departureTime();
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return res.value<BusReservation>().reservationFor().value<BusTrip>().departureTime();
    }
    if (JsonLd::isA<FlightReservation>(res)) {
        const auto flight = res.value<FlightReservation>().reservationFor().value<Flight>();
        if (flight.boardingTime().isValid()) {
            return flight.boardingTime();
        }
        return flight.departureTime();
    }
    if (JsonLd::isA<EventReservation>(res)) {
        const auto event = res.value<EventReservation>().reservationFor().value<Event>();
        if (event.doorTime().isValid()) {
            return event.doorTime();
        }
        return event.startDate();
    }
    if (JsonLd::isA<FoodEstablishmentReservation>(res)) {
        return res.value<FoodEstablishmentReservation>().startTime();
    }
    return {};
}

QDateTime TransferHelper::anchorTimeAfter(const QVariant &res)
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
    if (JsonLd::isA<EventReservation>(res)) {
        return res.value<EventReservation>().reservationFor().value<Event>().endDate();
    }
    if (JsonLd::isA<FoodEstablishmentReservation>(res)) {
        return res.value<FoodEstablishmentReservation>().endTime();
    }
    return {};
}
