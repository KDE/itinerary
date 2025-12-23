// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport
import org.kde.kpublictransport.ui as KPublicTransport
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
    property bool isCanceled: false
    required property var reservationFor

    spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
    enabled: !root.isCanceled

    Layout.fillWidth: true

    JourneySectionStopDelegateLineSegment {
        Layout.alignment: Qt.AlignTop
        implicitHeight: arrivalRow.height + Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
        lineColor: root.isCanceled ? Kirigami.Theme.disabledTextColor : root.departure.route.line.hasColor ? root.departure.route.line.color : Kirigami.Theme.textColor
        progress: root.progress
        isArrival: true
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
            id: arrivalRow
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

        KPublicTransport.ExpectedTimeLabel {
            stopover: root.arrival
            hasExpectedTime: root.arrival.hasExpectedArrivalTime
            visible: hasExpectedTime
            delay: root.arrival.arrivalDelay
            scheduledTime: delayed ? Localizer.formatTime(root.arrival, "scheduledArrivalTime") : ""
        }

        QQC2.Label {
            text: root.arrivalPlatform
            visible: text.length > 0
            color: root.arrivalPlatformChanged ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
            font.weight: root.arrivalPlatformChanged ? Font.DemiBold : Font.Normal
        }
    }
}
