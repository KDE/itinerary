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
            Layout.fillHeight: true
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

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            visible: root.arrival.hasExpectedArrivalTime

            Kirigami.Heading {
                level: 5
                text: oldTime.visible ? i18nc("duration of the delay e.g. Delayed 5 min", "Delayed %1", Localizer.formatDurationCustom(root.arrival.arrivalDelay * 60)) + ' - ' : i18nc("@info", "On time")
                color: oldTime.visible ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                font.weight: Font.DemiBold
            }

            QQC2.Label {
                id: oldTime

                opacity: 0.8
                visible: root.arrival.arrivalDelay * 60 > 1
                text: Localizer.formatTime(root.arrival, "scheduledArrivalTime")
                font.strikeout: true
            }
        }

        QQC2.Label {
            text: root.arrivalPlatform
            visible: text.length > 0
        }
    }
}
