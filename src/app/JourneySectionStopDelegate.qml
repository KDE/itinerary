/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport
import org.kde.itinerary
import "." as App

Item {
    id: root
    property var stop
    property bool isDeparture: false
    property bool isArrival: false

    readonly property bool isSameTime: stop.scheduledDepartureTime.getTime() == stop.scheduledArrivalTime.getTime()
    readonly property bool isSingleTime: !(stop.scheduledDepartureTime > 0 && stop.scheduledArrivalTime > 0)  || isSameTime

    // outbound progress overlay properties
    readonly property real leadingSegmentLength: lineSegment.leadingLineLength
    readonly property real trailingSegmentLength: lineSegment.trailingLineLength + (notesLabel.visible ? stopNotesLine.height : 0)
    // inbound progress overlay properties
    property alias leadingProgress: lineSegment.leadingProgress
    property real trailingProgress
    property alias stopoverPassed: lineSegment.showStop

    x: Kirigami.Units.gridUnit
    implicitHeight: layout.implicitHeight + Kirigami.Units.gridUnit * 2
    implicitWidth: ListView.view.width - x - Kirigami.Units.largeSpacing

    GridLayout {
        id: layout
        width: parent.width
        height: parent.height
        columns: 6
        rows: 3
        rowSpacing: 0
        JourneySectionStopDelegateLineSegment {
            id: lineSegment
            Layout.column: 0
            Layout.row: 0
            Layout.fillHeight: true
            Layout.rowSpan: 2

            isArrival: root.isArrival
            isDeparture: root.isDeparture
            lineColor: stop.route.line.hasColor ? stop.route.line.color : Kirigami.Theme.textColor
            hasStop: !isIntermediate || stop.disruptionEffect !== Disruption.NoService
            showStop: false
            trailingProgress: Math.min(1.0, (root.trailingProgress * root.trailingSegmentLength) / lineSegment.trailingLineLength)
        }

        QQC2.Label {
            id: arrivalTime
            Layout.column: 1
            Layout.row: 0
            Layout.rowSpan: isSingleTime ? 2 : 1
            Layout.alignment: isSingleTime ? Qt.AlignVCenter : Qt.AlignBottom
            text: Localizer.formatTime(stop, "scheduledArrivalTime")
            visible: stop.scheduledArrivalTime > 0 && !isSameTime
            font.strikeout: stop.disruptionEffect === Disruption.NoService
        }
        QQC2.Label {
            Layout.column: 2
            Layout.row: 0
            Layout.rowSpan: arrivalTime.Layout.rowSpan
            Layout.alignment: isSingleTime ? Qt.AlignVCenter : Qt.AlignBottom
            text: (stop.arrivalDelay >= 0 ? "+" : "") + stop.arrivalDelay
            color: stop.arrivalDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
            visible: stop.hasExpectedArrivalTime && !isSameTime && stop.disruptionEffect !== Disruption.NoService
        }

        QQC2.Label {
            Layout.column: 3
            Layout.row: 0
            Layout.rowSpan: arrivalTime.Layout.rowSpan
            Layout.fillWidth: true
            Layout.alignment: isSingleTime ? Qt.AlignVCenter : Qt.AlignBottom
            text: stop.stopPoint.name
            elide: Text.ElideRight
            font.bold: root.isDeparture || root.isArrival
            font.strikeout: stop.disruptionEffect === Disruption.NoService
        }

        QQC2.Label {
            id: departureTime
            Layout.column: 1
            Layout.row: isSingleTime ? 0 : 1
            Layout.rowSpan: isSingleTime ? 2 : 1
            Layout.alignment: isSingleTime ? Qt.AlignVCenter : Qt.AlignTop
            text: Localizer.formatTime(stop, "scheduledDepartureTime")
            visible: stop.scheduledDepartureTime > 0
            font.strikeout: stop.disruptionEffect === Disruption.NoService
        }

        QQC2.Label {
            Layout.column: 2
            Layout.row: departureTime.Layout.row
            Layout.rowSpan: departureTime.Layout.rowSpan
            Layout.alignment: isSingleTime ? Qt.AlignVCenter : Qt.AlignTop
            text: (stop.departureDelay >= 0 ? "+" : "") + stop.departureDelay
            color: stop.departureDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
            visible: departureTime.visible && stop.hasExpectedDepartureTime && stop.disruptionEffect !== Disruption.NoService
        }

        App.VehicleLoadIndicator {
            Layout.column: 3
            Layout.row: 1
            Layout.rowSpan: 1
            Layout.alignment: isSingleTime ? Qt.AlignVCenter : Qt.AlignTop
            loadInformation: stop.loadInformation
        }

        QQC2.Label {
            Layout.column: 4
            Layout.row: 0
            Layout.rowSpan: arrivalTime.Layout.rowSpan
            Layout.alignment: isSingleTime ? Qt.AlignVCenter : Qt.AlignBottom

            color: stop.hasExpectedPlatform ? (stop.platformChanged ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor) : Kirigami.Theme.textColor
            text: {
                const platform = stop.hasExpectedPlatform ? stop.expectedPlatform : stop.scheduledPlatform;

                if (platform.length === 0) {
                    return '';
                }

                switch (stop.route.line.mode) {
                    case Line.Train:
                    case Line.Funicular:
                    case Line.LocalTrain:
                    case Line.LongDistanceTrain:
                    case Line.Metro:
                    case Line.RailShuttle:
                    case Line.RapidTransit:
                    case Line.Tramway:
                        return i18nc("Abreviation of train platform", "Pl. %1", platform)
                    case Line.Ferry:
                        return i18nc("Ferry dock, if possible use an abreviation", "Dock %1", platform)
                    default:
                        return i18nc("Generic abreviation of platform", "Pl. %1", platform)
                }
            }
            visible: stop.disruptionEffect !== Disruption.NoService
        }

        QQC2.ToolButton {
            Layout.column: 5
            Layout.row: 0
            Layout.rowSpan: arrivalTime.Layout.rowSpan
            Layout.alignment: isSingleTime ? Qt.AlignVCenter : Qt.AlignBottom
            Layout.bottomMargin: isSingleTime ? 0 : Math.round(-height / 4)
            visible: stop.stopPoint.hasCoordinate && stop.disruptionEffect !== Disruption.NoService
            icon.name: "map-symbolic"
            text: i18n("Show location")
            display: QQC2.AbstractButton.IconOnly
            onClicked: {
                const args = {
                    coordinate: Qt.point(stop.stopPoint.longitude, stop.stopPoint.latitude),
                    placeName: stop.stopPoint.name
                };
                if (!isDeparture) {
                    args.arrivalPlatformName = stop.hasExpectedPlatform ? stop.expectedPlatform : stop.scheduledPlatform;
                    args.arrivalPlatformMode = PublicTransport.lineModeToPlatformMode(stop.route.line.mode);
                    args.arrivalPlatformIfopt = stop.stopPoint.identifier("ifopt");
                }
                if (!isArrival) {
                    args.departurePlatformName = stop.hasExpectedPlatform ? stop.expectedPlatform : stop.scheduledPlatform;
                    args.departurePlatformMode = PublicTransport.lineModeToPlatformMode(stop.route.line.mode);
                    args.departurePlatformIfopt = stop.stopPoint.identifier("ifopt");
                }

                // ensure the map page ends up on top
                if (applicationWindow().pageStack.layers.depth < 2)
                    applicationWindow().pageStack.push(indoorMapPage, args);
                else
                    applicationWindow().pageStack.layers.push(indoorMapPage, args);
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.visible: hovered
        }


        JourneySectionStopDelegateLineSegment {
            id: stopNotesLine
            Layout.column: 0
            Layout.row: 2
            Layout.fillHeight: true
            visible: notesLabel.visible
            lineColor: stop.route.line.hasColor ? stop.route.line.color : Kirigami.Theme.textColor
            hasStop: false
            showStop: false
            leadingProgress:  Math.max(0, ((root.trailingProgress * root.trailingSegmentLength) - lineSegment.trailingLineLength) / stopNotesLine.height)
            trailingProgress: stopNotesLine.leadingProgress
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
            color: Kirigami.Theme.disabledTextColor
        }
    }
}
