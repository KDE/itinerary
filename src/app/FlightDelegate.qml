/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.itinerary

TimelineDelegate {
    id: root

    function airportDisplayString(airport)
    {
        return airport.name ? airport.name : airport.iataCode;
    }

    function airportDisplayCode(airport)
    {
        return airport.iataCode ? airport.iataCode : airport.name;
    }

    function airplaneSeatString() {
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

    headerIconSource: ReservationHelper.defaultIconName(root.reservation)
    headerItem: RowLayout {
        QQC2.Label {
            id: headerLabel
            text: i18n("%1 %2 → %3",
                reservationFor.airline.iataCode + " " + reservationFor.flightNumber,
                airportDisplayCode(reservationFor.departureAirport),
                airportDisplayCode(reservationFor.arrivalAirport))
            color: root.headerTextColor
            elide: Text.ElideRight
            Layout.fillWidth: true
            Accessible.ignored: true
        }
        QQC2.Label {
            text: isNaN(reservationFor.boardingTime.getTime()) ?
                Localizer.formatTime(reservationFor, "departureTime") :
                Localizer.formatTime(reservationFor, "boardingTime")
            color: root.headerTextColor
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
        QQC2.Label {
            visible: text !== ""
            width: topLayout.width
            text: Localizer.formatAddressWithContext(reservationFor.departureAirport.address,
                                                     reservationFor.arrivalAirport.address,
                                                     Settings.homeCountryIsoCode)
        }

        TimelineDelegateSeatRow {
            width: topLayout.width

            TimelineDelegateSeatRowLabel {
                text: i18nc("flight departure terminal", "Terminal: <b>%1</b>", reservationFor.departureTerminal || "-")
            }
            Kirigami.Separator {
                Layout.fillHeight: true
            }
            TimelineDelegateSeatRowLabel {
                text: i18nc("flight departure gate", "Gate: <b>%1</b>", reservationFor.departureGate || "-")
            }
            Kirigami.Separator {
                Layout.fillHeight: true
            }
            TimelineDelegateSeatRowLabel {
                text: i18nc("flight seats", "Seat: <b>%1</b>", airplaneSeatString())
            }
        }

        QQC2.Label {
            text: i18nc("flight arrival, %1 is airport, %2 is time", "Arrival at %1: %2",
                airportDisplayString(reservationFor.arrivalAirport),
                Localizer.formatDateTime(reservationFor, "arrivalTime") + " ")
            color: Kirigami.Theme.textColor
            wrapMode: Text.WordWrap
            width: topLayout.width
        }
        QQC2.Label {
            visible: text !== ""
            width: topLayout.width
            text: Localizer.formatAddressWithContext(reservationFor.arrivalAirport.address,
                                                     reservationFor.departureAirport.address,
                                                     Settings.homeCountryIsoCode)
        }
    }

    Accessible.name: headerLabel.text
}
