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
    title: i18n("Flight")
    editor: Component {
        App.FlightEditor {
            reservation: root.reservation
            resId: root.resId
        }
    }

    GridLayout {
        id: grid
        width: root.width
        columns: 2

        QQC2.Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: reservationFor.airline.iataCode + " " + reservationFor.flightNumber
            horizontalAlignment: Qt.AlignHCenter
            font.bold: true
        }

        // ticket barcode
        App.TicketTokenDelegate {
            Layout.columnSpan: 2
            ticket: reservation.reservedTicket
        }

        // flight details
        QQC2.Label {
            text: i18n("Boarding time:")
        }
        QQC2.Label {
            text: Localizer.formatDateTime(reservationFor, "boardingTime")
        }
        QQC2.Label {
            text: i18n("Boarding group:")
        }
        QQC2.Label {
            text: reservation.boardingGroup
        }
        QQC2.Label {
            text: i18n("Seat:")
        }
        QQC2.Label {
            text: reservation.airplaneSeat
        }
        QQC2.Label {
            text: i18n("Airline:")
        }
        QQC2.Label {
            text: reservationFor.airline.name
        }

        Kirigami.Separator {
            Layout.columnSpan: 2
            Layout.fillWidth: true
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
            text: Localizer.formatDateTime(reservationFor, "departureTime")
        }
        QQC2.Label {
            text: reservationFor.departureAirport.name + " (" + reservationFor.departureAirport.iataCode + ")"
            Layout.columnSpan: 2
        }
        App.PlaceDelegate {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            place: reservationFor.departureAirport
        }
        QQC2.Label {
            text: i18n("Departure terminal:")
        }
        QQC2.Label {
            text: reservationFor.departureTerminal
        }
        QQC2.Label {
            text: i18n("Departure gate:")
        }
        QQC2.Label {
            text: reservationFor.departureGate
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
            text: Localizer.formatDateTime(reservationFor, "arrivalTime")
        }
        QQC2.Label {
            text: reservationFor.arrivalAirport.name + " (" + reservationFor.arrivalAirport.iataCode + ")"
            Layout.columnSpan: 2
        }
        App.PlaceDelegate {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            place: reservationFor.arrivalAirport
        }

        Kirigami.Separator {
            Layout.columnSpan: 2
            Layout.fillWidth: true
        }

        // booking details
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
