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

    function airportDisplayString(airport: var): string {
        return airport.name ? airport.name : airport.iataCode;
    }

    function airportDisplayCode(airport: var): string {
        return airport.iataCode ? airport.iataCode : airport.name;
    }

    function airplaneSeat(): string {
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

    readonly property bool hasSeat: {
        for (const resId of resIds) {
            if (ReservationManager.reservation(resId).airplaneSeat)
                return true;
        }
        return false;
    }

    contentItem: ColumnLayout {
        spacing: 0

        TimelineDelegateDepartureLayout {
            id: departureLayout

            reservationFor: root.reservationFor
            departure: root.departure
            progress: root.controller.progress
            departureName: root.airportDisplayString(root.reservationFor.departureAirport) + " " + root.airportDisplayCode(reservationFor.departureAirport)
            departureCountry: Localizer.formatCountryWithContext(root.reservationFor.departureAirport.address,
                                                     root.reservationFor.arrivalAirport.address,
                                                     Settings.homeCountryIsoCode)
            transportName: root.reservationFor.airline.iataCode + " " + root.reservationFor.flightNumber
            transportIcon: ReservationHelper.defaultIconName(root.reservation)

            QQC2.Label {
                visible: !isNaN(root.reservationFor.boardingTime.getTime())
                text: !isNaN(root.reservationFor.boardingTime.getTime()) ? i18nc("@info", "Boarding time: %1", Localizer.formatTime(root.reservationFor, "boardingTime")) : ''
                font.weight: Font.DemiBold
            }
        }

        TimelineDelegateSeatRow {
            route: root.departure.route
            hasSeat: root.hasSeat

            Component.onCompleted: progress = root.controller.progress;

            TimelineDelegateSeatRowLabel {
                text: i18nc("flight departure terminal", "Terminal: <b>%1</b>", root.reservationFor.departureTerminal || "-")
            }
            Kirigami.Separator {
                Layout.fillHeight: true
            }
            TimelineDelegateSeatRowLabel {
                text: i18nc("flight departure gate", "Gate: <b>%1</b>", root.reservationFor.departureGate || "-")
            }
            Kirigami.Separator {
                Layout.fillHeight: true
            }
            TimelineDelegateSeatRowLabel {
                text: i18nc("flight seats", "Seat: <b>%1</b>", root.airplaneSeat())
            }
        }

        TimelineDelegateArrivalLayout {
            arrival: root.arrival
            progress: root.controller.progress
            reservationFor: root.reservationFor
            arrivalName: root.airportDisplayString(root.reservationFor.arrivalAirport) + ' ' + root.airportDisplayCode(root.reservationFor.arrivalAirport)
            arrivalCountry: Localizer.formatCountryWithContext(root.reservationFor.arrivalAirport.address,
                                                     root.reservationFor.departureAirport.address,
                                                     Settings.homeCountryIsoCode)
        }
    }

    Accessible.name: i18n("%1 %2 → %3", root.reservationFor.airline.iataCode + " " + root.reservationFor.flightNumber,
                root.airportDisplayCode(root.reservationFor.departureAirport),
                root.airportDisplayCode(root.reservationFor.arrivalAirport))
}
