/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.0 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.DetailsPage {
    id: root
    title: i18n("Bus Ticket")

    GridLayout {
        id: grid
        width: root.width
        columns: 2

        QQC2.Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: reservation.reservationFor.busName + " " + reservation.reservationFor.busNumber
            horizontalAlignment: Qt.AlignHCenter
            font.bold: true
        }

        // ticket barcode
        App.TicketTokenDelegate {
            Layout.columnSpan: 2
            ticket: reservation.reservedTicket
        }

        // departure data
        QQC2.Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: i18n("Departure")
            horizontalAlignment: Qt.AlignHCenter
        }
        QQC2.Label {
            text: i18n("Departure time:")
        }
        QQC2.Label {
            text: Localizer.formatDateTime(reservation.reservationFor, "departureTime")
        }
        QQC2.Label {
            text: reservation.reservationFor.departureStation.name
            Layout.columnSpan: 2
        }
        App.PlaceDelegate {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            place: reservation.reservationFor.departureStation
        }

        Kirigami.Separator {
            Layout.columnSpan: 2
            Layout.fillWidth: true
        }

        // arrival data
        QQC2.Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: i18n("Arrival")
            horizontalAlignment: Qt.AlignHCenter
        }
        QQC2.Label {
            text: i18n("Arrival time:")
        }
        QQC2.Label {
            text: Localizer.formatDateTime(reservation.reservationFor, "arrivalTime")
        }
        QQC2.Label {
            text: reservation.reservationFor.arrivalStation.name
            Layout.columnSpan: 2
        }
        App.PlaceDelegate {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            place: reservation.reservationFor.arrivalStation
        }

        Kirigami.Separator {
            Layout.columnSpan: 2
            Layout.fillWidth: true
        }

        // seat reservation
        QQC2.Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: i18n("Seat")
            horizontalAlignment: Qt.AlignHCenter
        }
        QQC2.Label {
            text: i18n("Seat:")
        }
        QQC2.Label {
            text: reservation.reservedTicket.ticketedSeat.seatNumber
        }

        // booking details
        Kirigami.Separator {
            Layout.columnSpan: 2
            Layout.fillWidth: true
        }

        QQC2.Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: i18n("Booking")
            horizontalAlignment: Qt.AlignHCenter
        }
        QQC2.Label {
            text: i18n("Booking reference:")
        }
        QQC2.Label {
            text: reservation.reservationNumber
        }
        QQC2.Label {
            text: i18n("Under name:")
        }
        QQC2.Label {
            text: reservation.underName.name
        }
    }
}

