// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport
import org.kde.itinerary

RowLayout {
    id: root

    required property var arrival
    property var departure: arrival
    required property string arrivalName
    property string arrivalPlatform
    property bool arrivalPlatformChanged
    required property string arrivalCountry
    property real progress
    required property var reservationFor

    spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

    Layout.fillWidth: true

    ColumnLayout {
        spacing: 0
        JourneySectionStopDelegateLineSegment {
            Layout.fillHeight: true
            lineColor: root.departure.route.line.hasColor ? root.departure.route.line.color : Kirigami.Theme.textColor
            leadingProgress: root.progress > 0.99 ? 1 : 0
            trailingProgress: root.progress > 0.99 ? 1 : 0
            hasStop: false
        }
        JourneySectionStopDelegateLineSegment {
            Layout.minimumHeight: implicitWidth
            lineColor: root.departure.route.line.hasColor ? root.departure.route.line.color : Kirigami.Theme.textColor
            leadingProgress: root.progress > 0.99 ? 1 : 0
            trailingProgress: root.progress > 0.99 ? 1 : 0
            isArrival: true
        }
    }

    ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Layout.fillHeight: true
        Layout.fillWidth: true

        Kirigami.Separator {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                level: 3
                text: root.arrivalName
                elide: Text.ElideRight
                Layout.fillWidth: root.arrivalCountry.length === 0
            }

            QQC2.Label {
                visible: root.arrivalCountry.length > 0
                elide: Text.ElideRight
                text: '(' + root.arrivalCountry + ')'
                opacity: 0.8
                Layout.fillWidth: true
            }

            Kirigami.Heading {
                level: 2
                text: root.arrival.hasExpectedArrivalTime ? Localizer.formatTime(root.arrival, "expectedArrivalTime") : Localizer.formatTime(reservationFor, "arrivalTime")
            }
        }

        DelayRow {
            stopover: root.arrival
            visible: root.arrival.hasExpectedArrivalTime
            delay: root.arrival.arrivalDelay
            originalTime: Localizer.formatTime(root.arrival, "scheduledArrivalTime")
        }

        QQC2.Label {
            text: root.arrivalPlatform
            visible: text.length > 0
            color: root.arrivalPlatformChanged ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
            font.weight: root.arrivalPlatformChanged ? Font.DemiBold : Font.Normal
        }
    }
}
