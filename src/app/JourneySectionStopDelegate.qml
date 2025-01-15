/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary

Item {
    id: root
    property KPublicTransport.stopover stop
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

    implicitHeight: layout.implicitHeight + Kirigami.Units.gridUnit * 2
    implicitWidth: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin

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
            lineColor: root.stop.route.line.hasColor ? root.stop.route.line.color : Kirigami.Theme.textColor
            hasStop: !isIntermediate || root.stop.disruptionEffect !== KPublicTransport.Disruption.NoService
            showStop: false
            trailingProgress: Math.min(1.0, (root.trailingProgress * root.trailingSegmentLength) / lineSegment.trailingLineLength)
        }

        QQC2.Label {
            id: arrivalTime
            Layout.column: 1
            Layout.row: 0
            Layout.rowSpan: root.isSingleTime ? 2 : 1
            Layout.alignment: root.isSingleTime ? Qt.AlignVCenter : Qt.AlignBottom
            text: Localizer.formatTime(stop, "scheduledArrivalTime")
            visible: root.stop.scheduledArrivalTime > 0 && !root.isSameTime
            font.strikeout: root.stop.disruptionEffect === KPublicTransport.Disruption.NoService
        }
        QQC2.Label {
            Layout.column: 2
            Layout.row: 0
            Layout.rowSpan: arrivalTime.Layout.rowSpan
            Layout.alignment: root.isSingleTime ? Qt.AlignVCenter : Qt.AlignBottom
            text: (root.stop.arrivalDelay >= 0 ? "+" : "") + root.stop.arrivalDelay
            color: root.stop.arrivalDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
            visible: arrivalTime.visible && root.stop.hasExpectedArrivalTime && !root.isSameTime && root.stop.disruptionEffect !== KPublicTransport.Disruption.NoService
        }

        QQC2.Label {
            Layout.column: 3
            Layout.row: 0
            Layout.rowSpan: arrivalTime.Layout.rowSpan
            Layout.fillWidth: true
            Layout.alignment: root.isSingleTime ? Qt.AlignVCenter : Qt.AlignBottom
            text: root.stop.stopPoint.name
            elide: Text.ElideRight
            font.bold: root.isDeparture || root.isArrival
            font.strikeout: root.stop.disruptionEffect === KPublicTransport.Disruption.NoService
        }

        QQC2.Label {
            id: departureTime
            Layout.column: 1
            Layout.row: root.isSingleTime ? 0 : 1
            Layout.rowSpan: root.isSingleTime ? 2 : 1
            Layout.alignment: root.isSingleTime ? Qt.AlignVCenter : Qt.AlignTop
            text: Localizer.formatTime(stop, "scheduledDepartureTime")
            visible: root.stop.scheduledDepartureTime > 0
            font.strikeout: root.stop.disruptionEffect === KPublicTransport.Disruption.NoService
        }

        QQC2.Label {
            Layout.column: 2
            Layout.row: departureTime.Layout.row
            Layout.rowSpan: departureTime.Layout.rowSpan
            Layout.alignment: root.isSingleTime ? Qt.AlignVCenter : Qt.AlignTop
            text: (root.stop.departureDelay >= 0 ? "+" : "") + root.stop.departureDelay
            color: root.stop.departureDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
            visible: departureTime.visible && root.stop.hasExpectedDepartureTime && root.stop.disruptionEffect !== KPublicTransport.Disruption.NoService
        }

        KPublicTransport.OccupancyIndicator {
            Layout.column: 3
            Layout.row: 1
            Layout.rowSpan: 1
            Layout.alignment: root.isSingleTime ? Qt.AlignVCenter : Qt.AlignTop
            Layout.preferredHeight: Kirigami.Units.iconSizes.small
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
            occupancy: root.stop.maximumOccupancy
        }

        QQC2.Label {
            Layout.column: 4
            Layout.row: 0
            Layout.rowSpan: arrivalTime.Layout.rowSpan
            Layout.alignment: root.isSingleTime ? Qt.AlignVCenter : Qt.AlignBottom

            color: root.stop.hasExpectedPlatform ? (root.stop.platformChanged ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor) : Kirigami.Theme.textColor
            text: {
                const platform = root.stop.hasExpectedPlatform ? root.stop.expectedPlatform : root.stop.scheduledPlatform;

                if (platform.length === 0) {
                    return '';
                }

                switch (root.stop.route.line.mode) {
                    case KPublicTransport.Line.Train:
                    case KPublicTransport.Line.Funicular:
                    case KPublicTransport.Line.LocalTrain:
                    case KPublicTransport.Line.LongDistanceTrain:
                    case KPublicTransport.Line.Metro:
                    case KPublicTransport.Line.RailShuttle:
                    case KPublicTransport.Line.RapidTransit:
                    case KPublicTransport.Line.Tramway:
                        return i18nc("Abreviation of train platform", "Pl. %1", platform)
                    case KPublicTransport.Line.Ferry:
                        return i18nc("Ferry dock, if possible use an abreviation", "Dock %1", platform)
                    default:
                        return i18nc("Generic abreviation of platform", "Pl. %1", platform)
                }
            }
            visible: root.stop.disruptionEffect !== KPublicTransport.Disruption.NoService
        }

        QQC2.ToolButton {
            Layout.column: 5
            Layout.row: 0
            Layout.rowSpan: arrivalTime.Layout.rowSpan
            Layout.alignment: root.isSingleTime ? Qt.AlignVCenter : Qt.AlignBottom
            Layout.bottomMargin: root.isSingleTime ? 0 : Math.round(-height / 4)
            visible: root.stop.stopPoint.hasCoordinate && root.stop.disruptionEffect !== KPublicTransport.Disruption.NoService
            icon.name: "map-symbolic"
            text: i18n("Show location")
            display: QQC2.AbstractButton.IconOnly
            onClicked: {
                const args = {
                    coordinate: Qt.point(root.stop.stopPoint.longitude, root.stop.stopPoint.latitude),
                    placeName: root.stop.stopPoint.name
                };
                if (!root.isDeparture) {
                    args.arrivalPlatformName = root.stop.hasExpectedPlatform ? root.stop.expectedPlatform : root.stop.scheduledPlatform;
                    args.arrivalPlatformMode = PublicTransport.lineModeToPlatformMode(root.stop.route.line.mode);
                    args.arrivalPlatformIfopt = root.stop.stopPoint.identifier("ifopt");
                }
                if (!root.isArrival) {
                    args.departurePlatformName = root.stop.hasExpectedPlatform ? root.stop.expectedPlatform : root.stop.scheduledPlatform;
                    args.departurePlatformMode = PublicTransport.lineModeToPlatformMode(root.stop.route.line.mode);
                    args.departurePlatformIfopt = root.stop.stopPoint.identifier("ifopt");
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
            lineColor: root.stop.route.line.hasColor ? root.stop.route.line.color : Kirigami.Theme.textColor
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
            text: root.stop.notes.join("<br/>")
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            visible: root.stop.notes.length > 0 && !root.isDeparture
            font.italic: true
            onLinkActivated: Qt.openUrlExternally(link)
            color: Kirigami.Theme.disabledTextColor
        }
    }
}
