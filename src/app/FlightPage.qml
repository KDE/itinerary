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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.DetailsPage {
    id: root
    title: i18n("Flight")
    editor: Component {
        App.FlightEditor {
            resIds: root.resIds
        }
    }

    Kirigami.FormLayout {
        width: root.width
        Component.onCompleted: Util.fixFormLayoutTouchTransparency(this)

        QQC2.Label {
            Kirigami.FormData.isSection: true
            text: reservationFor.airline.iataCode + " " + reservationFor.flightNumber
            horizontalAlignment: Qt.AlignHCenter
            font.bold: true
        }

        // ticket barcode
        App.TicketTokenDelegate {
            Kirigami.FormData.isSection: true
            resIds: root.resIds
        }

        // flight details
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Flight")
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Boarding:")
            text: Localizer.formatDateTime(reservationFor, "boardingTime")
            visible: reservationFor.boardingTime > 0

        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Boarding group:")
            text: reservation.boardingGroup
            visible: reservation.boardingGroup.length > 0

        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Seat:")
            text: reservation.airplaneSeat
            visible: reservation.airplaneSeat.length > 0

        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Airline:")
            text: reservationFor.airline.name
            visible: reservationFor.airline.name.length > 0
        }


        // departure data
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Departure")
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Time:")
            text: Localizer.formatDateTime(reservationFor, "departureTime")
            visible: reservationFor.departureTime > 0
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Airport:")
            text: reservationFor.departureAirport.name + " (" + reservationFor.departureAirport.iataCode + ")"
            visible: reservationFor.departureAirport.name.length > 0
        }
        App.PlaceDelegate {
            place: reservationFor.departureAirport
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Terminal:")
            text: reservationFor.departureTerminal
            visible: reservationFor.departureTerminal.length > 0
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Gate:")
            text: reservationFor.departureGate
            visible: reservationFor.departureGate.length > 0
        }


        // arrival data
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Arrival")
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Time:")
            text: Localizer.formatDateTime(reservationFor, "arrivalTime")
            visible: reservationFor.arrivalTime > 0
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Airport:")
            text: reservationFor.arrivalAirport.name + " (" + reservationFor.arrivalAirport.iataCode + ")"
            visible: reservationFor.arrivalAirport.name.length > 0
        }
        App.PlaceDelegate {
            place: reservationFor.arrivalAirport
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Terminal:")
            text: reservationFor.arrivalTerminal
            visible: reservationFor.arrivalTerminal.length > 0
        }

        // booking details
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Booking")
        }
        Repeater {
            model: resIds.length * 2
            delegate: QQC2.Label {
                property var res: _reservationManager.reservation(resIds[Math.floor(index/2)]);
                Kirigami.FormData.label: index % 2 == 0 ? i18n("Under name:") : i18n("Reference:")
                text: index % 2 == 0 ? res.underName.name : res.reservationNumber
            }
        }
    }
}
