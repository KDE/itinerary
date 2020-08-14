/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
