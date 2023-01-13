/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

App.TimelineDelegate {
    id: root
    headerIconSource: departure.route.line.mode == Line.Unknown ? "qrc:///images/bus.svg" : PublicTransport.lineModeIcon(departure.route.line.mode)
    headerItem: RowLayout {
        QQC2.Label {
            id: headerLabel
            text: {
                if (reservationFor.busName || reservationFor.busNumber ) {
                    return reservationFor.busName + " " + reservationFor.busNumber
                }
                return i18n("%1 to %2", reservationFor.departureBusStop.name, reservationFor.arrivalBusStop.name);
            }
            color: Kirigami.Theme.textColor
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
            Layout.fillWidth: true
            Accessible.ignored: true
        }
        QQC2.Label {
            text: Localizer.formatTime(reservationFor, "departureTime")
            color: Kirigami.Theme.textColor
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
        }
        QQC2.Label {
            text: (departure.departureDelay >= 0 ? "+" : "") + departure.departureDelay
            color: (departure.departureDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
            visible: departure.hasExpectedDepartureTime
            Accessible.ignored: !visible
        }
    }

    contentItem: Column {
        id: topLayout
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            text: i18n("From: %1", reservationFor.departureBusStop.name)
            color: Kirigami.Theme.textColor
            width: topLayout.width
        }
        App.PlaceDelegate {
            place: reservationFor.departureBusStop
            controller: root.controller
            isRangeBegin: true
            width: topLayout.width
            showButtons: false
        }
        Kirigami.Separator {
            width: topLayout.width
        }
        QQC2.Label {
            text: i18n("To: %1", reservationFor.arrivalBusStop.name)
            color: Kirigami.Theme.textColor
            width: topLayout.width
        }
        App.PlaceDelegate {
            place: reservationFor.arrivalBusStop
            controller: root.controller
            isRangeEnd: true
            width: topLayout.width
            showButtons: false
        }
        Row {
            width: topLayout.width
            spacing: Kirigami.Units.smallSpacing
            QQC2.Label {
                text: i18n("Arrival time: %1", Localizer.formatDateTime(reservationFor, "arrivalTime"))
                color: Kirigami.Theme.textColor
                wrapMode: Text.WordWrap
                visible: reservationFor.arrivalTime > 0
            }
            QQC2.Label {
                text: (arrival.arrivalDelay >= 0 ? "+" : "") + arrival.arrivalDelay
                color: (arrival.arrivalDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                visible: arrival.hasExpectedArrivalTime
                Accessible.ignored: !visible
            }
        }
    }

    onClicked: showDetailsPage(busDetailsPage, root.batchId)
    Accessible.name: headerLabel.text
}
