/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.12 as Kirigami
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

    contentItem: Column {
        id: topLayout
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            text: i18nc("flight departure, %1 is airport, %2 is time", "Departure from %1: %2",
                airportDisplayString(reservationFor.departureAirport),
                Localizer.formatTime(reservationFor, "departureTime") + " ")
            color: Kirigami.Theme.textColor
            wrapMode: Text.WordWrap
            width: topLayout.width
        }
        App.PlaceDelegate {
            place: reservationFor.departureAirport
            controller: root.controller
            isRangeBegin: true
            width: topLayout.width
        }
        QQC2.Label {
            text: i18n("Terminal: %1  Gate: %2  Seat: %3",
                reservationFor.departureTerminal ? reservationFor.departureTerminal : "-",
                reservationFor.departureGate ? reservationFor.departureGate : "-", seatString())
            color: Kirigami.Theme.textColor
        }

        Kirigami.Separator {
            width: topLayout.width
        }

        QQC2.Label {
            text: i18nc("flight arrival, %1 is airport, %2 is time", "Arrival at %1: %2",
                airportDisplayString(reservationFor.arrivalAirport),
                Localizer.formatDateTime(reservationFor, "arrivalTime") + " ")
            color: Kirigami.Theme.textColor
            wrapMode: Text.WordWrap
            width: topLayout.width
        }
        App.PlaceDelegate {
            place: reservationFor.arrivalAirport
            controller: root.controller
            isRangeEnd: true
            width: topLayout.width
        }
    }

    Component {
        id: detailsComponent
        App.FlightPage {
            batchId: root.batchId
        }
    }

    onClicked: showDetails(detailsComponent)
}
