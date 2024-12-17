// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport.ui
import org.kde.itinerary

RowLayout {
    id: root

    required property var departure
    required property var departureName
    required property string departureCountry
    property string departurePlatform
    property string transportName
    property string transportIcon
    property real progress
    required property var reservationFor

    default property alias _content: innerLayout.children

    spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

    Layout.fillWidth: true

    ColumnLayout {
        spacing: 0

        JourneySectionStopDelegateLineSegment {
            Layout.minimumHeight: implicitWidth
            lineColor: root.departure.route.line.hasColor ? root.departure.route.line.color : Kirigami.Theme.textColor
            isDeparture: true

            leadingProgress: root.progress > 0 ? 1 : 0
            trailingProgress: root.progress > 0 ? 1 : 0
        }

        JourneySectionStopDelegateLineSegment {
            lineColor: root.departure.route.line.hasColor ? root.departure.route.line.color : Kirigami.Theme.textColor
            hasStop: false
            leadingProgress: root.progress > 0 ? 1 : 0
            trailingProgress: root.progress > 0 ? 1 : 0

            Layout.fillHeight: true
        }
    }

    ColumnLayout {
        id: innerLayout

        spacing: Kirigami.Units.smallSpacing

        Layout.bottomMargin: Kirigami.Units.largeSpacing
        Layout.fillHeight: true
        Layout.fillWidth: true

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            Kirigami.Heading {
                level: 2
                text: root.departureName
                elide: Text.ElideRight
                Layout.fillWidth: root.departureCountry.length === 0
            }

            QQC2.Label {
                visible: root.departureCountry.length > 0
                elide: Text.ElideRight
                text: '(' + root.departureCountry + ')'
                opacity: 0.8
                Layout.fillWidth: true
            }

            Kirigami.Heading {
                text: departure.hasExpectedDepartureTime ? Localizer.formatTime(root.departure, "expectedDepartureTime") : Localizer.formatTime(reservationFor, "departureTime")
            }
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            TransportNameControl {
                line: root.departure.route.line
                mode: JourneySection.PublicTransport
                iconName: root.transportIcon
                lineName: root.transportName
            }

            QQC2.Label {
                text: departure.route.direction ? i18nc("Direction of the transport mode", "To %1", departure.route.direction) : ""
                visible: departure.route.direction
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        DelayRow {
            visible: root.departure.hasExpectedDepartureTime
            stopover: root.departure
            delay: root.departure.departureDelay
            originalTime: Localizer.formatTime(root.departure, "scheduledDepartureTime")
        }

        QQC2.Label {
            text: root.departurePlatform
            visible: text.length > 0
        }
    }
}
