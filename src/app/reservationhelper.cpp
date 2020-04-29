/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

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

#include "reservationhelper.h"

#include <KItinerary/BusTrip>
#include <KItinerary/Flight>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

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
