// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

App.TimelineDelegate {
    id: root
    headerIcon {
        source: departure.route.line.mode == Line.Unknown ? "qrc:///images/train.svg" : PublicTransport.lineIcon(departure.route.line)
        isMask: !departure.route.line.hasLogo && !departure.route.line.hasModeLogo
    }
    headerItem: RowLayout {
        QQC2.Label {
            id: headerLabel
            text: {
                if (reservationFor.trainName || reservationFor.trainNumber) {
                    return reservationFor.trainName + " " + reservationFor.trainNumber
                }
                return i18n("%1 to %2", reservationFor.departureStation.name, reservationFor.arrivalStation.name);
            }
            color: root.headerTextColor
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
            elide: Text.ElideRight
            Layout.fillWidth: true
            Accessible.ignored: true
        }
        QQC2.Label {
            text: Localizer.formatTime(reservationFor, "departureTime")
            color: root.headerTextColor
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
            text: i18n("Departure from %1 on platform %2",
                reservationFor.departureStation.name,
                departure.hasExpectedPlatform ? departure.expectedPlatform :
                reservationFor.departurePlatform ? reservationFor.departurePlatform : "-")
            color: Kirigami.Theme.textColor
            wrapMode: Text.WordWrap
            width: topLayout.width
        }
        App.PlaceDelegate {
            place: reservationFor.departureStation
            isRangeBegin: true
            width: topLayout.width
            controller: root.controller
            showButtons: false
        }

        // TODO reserved seat

        Kirigami.Separator {
            width: topLayout.width
        }
        QQC2.Label {
            text: i18n("Arrival at %1 on platform %2",
                reservationFor.arrivalStation.name,
                arrival.hasExpectedPlatform ? arrival.expectedPlatform :
                reservationFor.arrivalPlatform ? reservationFor.arrivalPlatform : "-")
            color: Kirigami.Theme.textColor
            wrapMode: Text.WordWrap
            width: topLayout.width
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
        App.PlaceDelegate {
            place: reservationFor.arrivalStation
            isRangeEnd: true
            width: topLayout.width
            controller: root.controller
            showButtons: false
        }
    }

    onClicked: showDetailsPage(trainDetailsPage, root.batchId)
    Accessible.name: headerLabel.text
}
