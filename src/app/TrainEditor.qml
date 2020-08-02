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

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.EditorPage {
    id: root
    title: i18n("Edit Train Reservation")

    function save(resId, reservation) {
        var trip = reservation.reservationFor;
        trip.departurePlatform = departurePlatform.text;
        trip.arrivalPlatform = arrivalPlatform.text;

        var seat = reservation.reservedTicket.ticketedSeat;
        seat.seatSection = coachEdit.text;
        seat.seatNumber = seatEdit.text;
        seat.seatingType = classEdit.text;

        var ticket = reservation.reservedTicket;
        ticket.ticketedSeat = seat;

        var newRes = reservation;
        newRes.reservationFor = trip;
        newRes.reservedTicket = ticket;

        ReservationManager.updateReservation(resId, newRes);
    }

    Kirigami.FormLayout {
        width: parent.width

        // departure data
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Departure")
        }

        // TODO time
        QQC2.Label {
            Kirigami.FormData.label: i18n("Station:")
            text: reservationFor.departureStation.name
        }
        QQC2.TextField {
            id: departurePlatform
            Kirigami.FormData.label: i18n("Platform:")
            text: reservationFor.departurePlatform
        }

        // arrival data
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Arrival")
        }

        // TODO time
        QQC2.Label {
            Kirigami.FormData.label: i18n("Station:")
            text: reservationFor.arrivalStation.name
        }
        QQC2.TextField {
            id: arrivalPlatform
            Kirigami.FormData.label: i18n("Platform:")
            text: reservationFor.arrivalPlatform
        }

        // TODO the below is per reservation, not per batch, so add a selector for that!
        // seat reservation
        Kirigami.Separator {
            Kirigami.FormData.label: i18n("Seat")
            Kirigami.FormData.isSection: true
        }
        QQC2.TextField {
            id: coachEdit
            Kirigami.FormData.label: i18n("Coach:")
            text: reservation.reservedTicket.ticketedSeat.seatSection
        }
        QQC2.TextField {
            id: seatEdit
            Kirigami.FormData.label: i18n("Seat:")
            text: reservation.reservedTicket.ticketedSeat.seatNumber
        }
        QQC2.TextField {
            id: classEdit
            Kirigami.FormData.label: i18n("Class:")
            text: reservation.reservedTicket.ticketedSeat.seatingType
        }

        // booking details
        Kirigami.Separator {
            Kirigami.FormData.label: i18n("Booking")
            Kirigami.FormData.isSection: true
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Reference:")
            text: reservation.reservationNumber
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Under name:")
            text: reservation.underName.name
        }
    }
}
