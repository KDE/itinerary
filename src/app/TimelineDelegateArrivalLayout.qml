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
    required property string arrivalName
    property string arrivalPlatform
    required property string arrivalCountry
    property real progress
    required property var reservationFor
    required property int depTimeWidth

    spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

    Layout.fillWidth: true

    ColumnLayout {
        spacing: 0
        JourneySectionStopDelegateLineSegment {
            visible: arrivalCountryLayout.visible
            Layout.fillHeight: true
            lineColor: root.arrival.route.line.hasColor ? root.arrival.route.line.color : Kirigami.Theme.textColor
            leadingProgress: root.progress === 0.99 ? 1 : 0
            trailingProgress: root.progress === 0.99 ? 1 : 0
            hasStop: false
        }
        JourneySectionStopDelegateLineSegment {
            Layout.fillHeight: true
            lineColor: root.arrival.route.line.hasColor ? root.arrival.route.line.color : Kirigami.Theme.textColor
            leadingProgress: root.progress === 0.99 ? 1 : 0
            trailingProgress: root.progress === 0.99 ? 1 : 0
            isArrival: true
        }
    }

    ColumnLayout {
        spacing: 0

        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillHeight: true
        Layout.fillWidth: true

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Layout.fillHeight: true
            Layout.fillWidth: true

            RowLayout {
                spacing: Kirigami.Units.smallSpacing

                Layout.minimumWidth: root.depTimeWidth + Kirigami.Units.largeSpacing * 3.5

                QQC2.Label {
                    text: Localizer.formatTime(reservationFor, "arrivalTime")
                }

                QQC2.Label {
                    text: (arrival.arrivalDelay >= 0 ? "+" : "") + arrival.arrivalDelay
                    color: (arrival.arrivalDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                    visible: arrival.hasExpectedArrivalTime
                    Accessible.ignored: !visible
                }
            }

            QQC2.Label {
                Layout.fillWidth: true
                font.bold: true
                text: root.arrivalName
                elide: Text.ElideRight
            }

            QQC2.Label {
                Layout.alignment: Qt.AlignRight
                text: root.arrivalPlatform
                visible: root.arrivalPlatform.length > 0
            }
        }

        RowLayout {
            id: arrivalCountryLayout

            spacing: Kirigami.Units.smallSpacing
            visible: arrivalCountryLabel.text.length > 0

            Item {
                Layout.minimumWidth: root.depTimeWidth + Kirigami.Units.largeSpacing * 3.5
            }

            QQC2.Label {
                id: arrivalCountryLabel

                text: root.arrivalCountry
                Layout.fillWidth: true
            }
        }
    }
}
