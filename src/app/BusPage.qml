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
    title: i18n("Bus Ticket")

    Kirigami.FormLayout {
        width: root.width
        Component.onCompleted: Util.fixFormLayoutTouchTransparency(this)

        QQC2.Label {
            Kirigami.FormData.isSection: true
            text: reservationFor.busName + " " + reservationFor.busNumber
            horizontalAlignment: Qt.AlignHCenter
            font.bold: true
        }

        // ticket barcode
        App.TicketTokenDelegate {
            Kirigami.FormData.isSection: true
            resIds: _reservationManager.reservationsForBatch(root.batchId)
        }

        // departure data
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Departure")
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Time:")
            text: Localizer.formatDateTime(reservationFor, "departureTime")
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Station:")
            text: reservationFor.departureBusStop.name
        }
        App.PlaceDelegate {
            place: reservationFor.departureBusStop
        }

        // arrival data
        Kirigami.Separator {
            Kirigami.FormData.label: i18n("Arrival")
            Kirigami.FormData.isSection: true
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Time:")
            text: Localizer.formatDateTime(reservationFor, "arrivalTime")
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Station:")
            text: reservationFor.arrivalBusStop.name
        }
        App.PlaceDelegate {
            place: reservationFor.arrivalBusStop
        }

        // seat reservation
        Kirigami.Separator {
            Kirigami.FormData.label: i18n("Seat")
            Kirigami.FormData.isSection: true
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Seat:")
            text: reservation.reservedTicket.ticketedSeat.seatNumber
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
