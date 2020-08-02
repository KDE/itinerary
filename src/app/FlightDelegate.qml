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
import org.kde.itinerary 1.0
import "." as App

App.TimelineDelegate {
    id: root

    function airportDisplayString(airport)
    {
        return airport.name ? airport.name : airport.iataCode;
    }

    function airportDisplayCode(airport)
    {
        return airport.iataCode ? airport.iataCode : airport.name;
    }

    function seatString() {
        var s = new Array();
        for (var i = 0; i < root.resIds.length; ++i) {
            var res = ReservationManager.reservation(root.resIds[i]);
            if (res.airplaneSeat)
                s.push(res.airplaneSeat);
        }
        if (s.length === 0)
            return "-";
        return s.join(", ");
    }

    headerIconSource: "qrc:///images/flight.svg"
    headerItem: RowLayout {
        QQC2.Label {
            text: i18n("%1 %2 → %3",
                reservationFor.airline.iataCode + " " + reservationFor.flightNumber,
                airportDisplayCode(reservationFor.departureAirport),
                airportDisplayCode(reservationFor.arrivalAirport))
            color: Kirigami.Theme.textColor
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: isNaN(reservationFor.boardingTime.getTime()) ?
                Localizer.formatTime(reservationFor, "departureTime") :
                Localizer.formatTime(reservationFor, "boardingTime")
            color: Kirigami.Theme.textColor
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
        }
    }

    contentItem: ColumnLayout {
        id: topLayout

        QQC2.Label {
            text: i18nc("flight departure, %1 is airport, %2 is time", "Departure from %1: %2",
                airportDisplayString(reservationFor.departureAirport),
                Localizer.formatTime(reservationFor, "departureTime") + " ")
            color: Kirigami.Theme.textColor
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        App.PlaceDelegate {
            place: reservationFor.departureAirport
            controller: root.controller
            isRangeBegin: true
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: i18n("Terminal: %1  Gate: %2  Seat: %3",
                reservationFor.departureTerminal ? reservationFor.departureTerminal : "-",
                reservationFor.departureGate ? reservationFor.departureGate : "-", seatString())
            color: Kirigami.Theme.textColor
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        QQC2.Label {
            text: i18nc("flight arrival, %1 is airport, %2 is time", "Arrival at %1: %2",
                airportDisplayString(reservationFor.arrivalAirport),
                Localizer.formatDateTime(reservationFor, "arrivalTime") + " ")
            color: Kirigami.Theme.textColor
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        App.PlaceDelegate {
            place: reservationFor.arrivalAirport
            controller: root.controller
            isRangeEnd: true
            Layout.fillWidth: true
        }
    }

    Component {
        id: detailsComponent
        App.FlightPage {
            batchId: root.batchId
        }
    }

    onClicked: applicationWindow().pageStack.push(detailsComponent);
}
