/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

Item {
    property var stop
    property bool isDeparture: false
    property bool isArrival: false
    readonly property bool isIntermediate: !isDeparture && !isArrival

    readonly property bool isSameTime: stop.scheduledDepartureTime.getTime() == stop.scheduledArrivalTime.getTime()
    readonly property bool isSingleTime: !(stop.scheduledDepartureTime > 0 && stop.scheduledArrivalTime > 0)  || isSameTime

    x: Kirigami.Units.gridUnit
    implicitHeight: layout.implicitHeight + Kirigami.Units.gridUnit * 2
    implicitWidth: ListView.view.width - x - Kirigami.Units.largeSpacing

    GridLayout {
        id: layout
        width: parent.width
        height: parent.height
        columns: 7
        rows: 3
        Item {
            id: lineSegment
            Layout.column: 0
            Layout.row: 0
            Layout.fillHeight: true
            Layout.rowSpan: 2

            property color lineColor: stop.route.line.hasColor ? stop.route.line.color : Kirigami.Theme.textColor
            property int lineWidth: Kirigami.Units.smallSpacing

            implicitWidth: lineSegment.lineWidth * 5

            Rectangle {
                x: 2 * lineSegment.lineWidth
                width: lineSegment.lineWidth
                color: lineSegment.lineColor
                height: parent.height / 2
                visible: !isDeparture
            }
            Rectangle {
                x: 2 * lineSegment.lineWidth
                y: height
                width: lineSegment.lineWidth
                color: lineSegment.lineColor
                height: parent.height / 2
                visible: !isArrival
            }

            Rectangle {
                radius: width / 2
                width: lineSegment.lineWidth * (isIntermediate ? 4 : 5)
                height: width
                border {
                    width: lineSegment.lineWidth
                    color: lineSegment.lineColor
                }
                color: Kirigami.Theme.backgroundColor
                anchors.centerIn: parent
                visible: !isIntermediate || stop.disruptionEffect != Disruption.NoService
            }
        }

        QQC2.Label {
            id: arrivalTime
            Layout.column: 1
            Layout.row: 0
            Layout.rowSpan: isSingleTime ? 2 : 1
            Layout.alignment: isSingleTime ? Qt.AlignVCenter : Qt.AlignBottom
            text: Localizer.formatTime(stop, "scheduledArrivalTime")
            visible: stop.scheduledArrivalTime > 0 && !isSameTime
        }
        QQC2.Label {
            Layout.column: 2
            Layout.row: 0
            Layout.rowSpan: arrivalTime.Layout.rowSpan
            Layout.alignment: isSingleTime ? Qt.AlignVCenter : Qt.AlignBottom
            text: (stop.arrivalDelay >= 0 ? "+" : "") + stop.arrivalDelay
            color: stop.arrivalDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
            visible: stop.hasExpectedArrivalTime && !isSameTime
            verticalAlignment: Qt.AlignVCenter
        }

        QQC2.Label {
            Layout.column: 3
            Layout.row: 0
            Layout.rowSpan: 2
            Layout.fillWidth: true
            Layout.fillHeight: true
            text: stop.stopPoint.name
            verticalAlignment: Qt.AlignVCenter
            enabled: stop.disruptionEffect != Disruption.NoService
        }

        QQC2.Label {
            id: departureTime
            Layout.column: 1
            Layout.row: isSingleTime ? 0 : 1
            Layout.rowSpan: isSingleTime ? 2 : 1
            Layout.alignment: isSingleTime ? Qt.AlignVCenter : Qt.AlignTop
            text: Localizer.formatTime(stop, "scheduledDepartureTime")
            visible: stop.scheduledDepartureTime > 0
        }
        QQC2.Label {
            Layout.column: 2
            Layout.row: departureTime.Layout.row
            Layout.rowSpan: departureTime.Layout.rowSpan
            Layout.alignment: isSingleTime ? Qt.AlignVCenter : Qt.AlignTop
            text: (stop.departureDelay >= 0 ? "+" : "") + stop.departureDelay
            color: stop.departureDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
            visible: departureTime.visible && stop.hasExpectedDepartureTime
        }

        App.VehicleLoadIndicator {
            Layout.column: 4
            Layout.row: 0
            Layout.rowSpan: 2
            Layout.alignment: Qt.AlignVCenter
            loadInformation: stop.loadInformation
        }

        QQC2.Label {
            Layout.column: 5
            Layout.row: 0
            Layout.rowSpan: 2
            Layout.fillHeight: true
            verticalAlignment: Qt.AlignVCenter
            text: stop.hasExpectedPlatform ? stop.expectedPlatform : stop.scheduledPlatform
            color: stop.hasExpectedPlatform ? (stop.platformChanged ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor) : Kirigami.Theme.textColor
        }

        QQC2.ToolButton {
            Layout.column: 6
            Layout.row: 0
            Layout.rowSpan: 2
            Layout.alignment: Qt.AlignVCenter
            visible: stop.stopPoint.hasCoordinate
            icon.name: "map-symbolic"
            onClicked: {
                var args = {
                    coordinate: Qt.point(stop.stopPoint.longitude, stop.stopPoint.latitude),
                    placeName: stop.stopPoint.name
                };
                if (!isDeparture) {
                    args.arrivalPlatformName = stop.hasExpectedPlatform ? stop.expectedPlatform : stop.scheduledPlatform;
                }
                if (!isArrival) {
                    args.departurePlatformName = stop.hasExpectedPlatform ? stop.expectedPlatform : stop.scheduledPlatform;
                }
                applicationWindow().pageStack.push(indoorMapPage, args);
            }
        }

        // intermediate stop notes
        Item {
            Layout.column: 0
            Layout.row: 2
            Layout.fillHeight: true
            implicitWidth: lineSegment.implicitWidth
            visible: notesLabel.visible

            Rectangle {
                y: -layout.rowSpacing
                x: 2 * lineSegment.lineWidth
                width: lineSegment.lineWidth
                color: lineSegment.lineColor
                height: parent.height - y
            }
        }

        QQC2.Label {
            id: notesLabel
            Layout.column: 3
            Layout.row: 2
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.fillHeight: true
            verticalAlignment: Qt.AlignTop
            text: stop.notes.join("<br/>")
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            visible: stop.notes.length > 0 && !isDeparture
            font.italic: true
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }
}
