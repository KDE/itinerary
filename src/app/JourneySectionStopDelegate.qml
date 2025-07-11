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
    /** Highlight this entry, e.g. because it's the next stop or the departure/arrival. */
    property bool highlight: root.isDeparture || root.isArrival

    readonly property bool isSameTime: stop.scheduledDepartureTime.getTime() == stop.scheduledArrivalTime.getTime()
    readonly property bool isSingleTime: !(stop.scheduledDepartureTime > 0 && stop.scheduledArrivalTime > 0)  || isSameTime


    property real progress
    property alias stopoverPassed: lineSegment.showStop

    property real topPadding: 0

    implicitHeight: layout.implicitHeight + Kirigami.Units.gridUnit * 2 + root.topPadding
    implicitWidth: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin

    GridLayout {
        id: layout
        width: parent.width
        height: parent.height
        columns: 6
        rows: 3
        rowSpacing: 0
        y: root.topPadding
        JourneySectionStopDelegateLineSegment {
            id: lineSegment
            Layout.column: 0
            Layout.row: 0
            Layout.fillHeight: true
            Layout.rowSpan: 3

            isArrival: root.isArrival
            isDeparture: root.isDeparture
            lineColor: root.stop.route.line.hasColor ? root.stop.route.line.color : Kirigami.Theme.textColor
            hasStop: root.stop.disruptionEffect !== KPublicTransport.Disruption.NoService &&
                (root.stop.pickupType !== KPublicTransport.PickupDropoff.NotAllowed || root.stop.dropoffType !== KPublicTransport.PickupDropoff.NotAllowed)
            showStop: false
            stopY: arrivalTime.height / 2
            progress: root.progress
        }

        QQC2.Label {
            id: arrivalTime
            Layout.column: 1
            Layout.row: 0
            Layout.alignment: Qt.AlignTop
            text: Localizer.formatTime(stop, "scheduledArrivalTime")
            visible: root.stop.scheduledArrivalTime > 0 && !root.isSameTime
            font.strikeout: root.stop.disruptionEffect === KPublicTransport.Disruption.NoService
            opacity: root.stop.pickupType === KPublicTransport.PickupDropoff.NotAllowed && root.stop.dropoffType === KPublicTransport.PickupDropoff.NotAllowed ? 0.5 : 1.0
        }
        QQC2.Label {
            Layout.column: 2
            Layout.row: 0
            Layout.alignment:  Qt.AlignTop
            text: (root.stop.arrivalDelay >= 0 ? "+" : "") + root.stop.arrivalDelay
            color: root.stop.arrivalDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
            visible: arrivalTime.visible && root.stop.hasExpectedArrivalTime && !root.isSameTime && root.stop.disruptionEffect !== KPublicTransport.Disruption.NoService
            opacity: root.stop.pickupType === KPublicTransport.PickupDropoff.NotAllowed && root.stop.dropoffType === KPublicTransport.PickupDropoff.NotAllowed ? 0.5 : 1.0
        }

        QQC2.Label {
            Layout.column: 3
            Layout.row: 0
            Layout.fillWidth: true
            Layout.alignment:  Qt.AlignTop
            text: root.stop.stopPoint.name
            elide: Text.ElideRight
            font.bold: root.highlight
            font.strikeout: root.stop.disruptionEffect === KPublicTransport.Disruption.NoService
            opacity: root.stop.pickupType === KPublicTransport.PickupDropoff.NotAllowed && root.stop.dropoffType === KPublicTransport.PickupDropoff.NotAllowed ? 0.5 : 1.0
        }

        QQC2.Label {
            id: departureTime
            Layout.column: 1
            Layout.row: root.isSingleTime ? 0 : 1
            Layout.alignment:  Qt.AlignTop
            text: Localizer.formatTime(stop, "scheduledDepartureTime")
            visible: root.stop.scheduledDepartureTime > 0
            font.strikeout: root.stop.disruptionEffect === KPublicTransport.Disruption.NoService
            opacity: root.stop.pickupType === KPublicTransport.PickupDropoff.NotAllowed && root.stop.dropoffType === KPublicTransport.PickupDropoff.NotAllowed ? 0.5 : 1.0
        }

        QQC2.Label {
            Layout.column: 2
            Layout.row: departureTime.Layout.row
            Layout.alignment:  Qt.AlignTop
            text: (root.stop.departureDelay >= 0 ? "+" : "") + root.stop.departureDelay
            color: root.stop.departureDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
            visible: departureTime.visible && root.stop.hasExpectedDepartureTime && root.stop.disruptionEffect !== KPublicTransport.Disruption.NoService
            opacity: root.stop.pickupType === KPublicTransport.PickupDropoff.NotAllowed && root.stop.dropoffType === KPublicTransport.PickupDropoff.NotAllowed ? 0.5 : 1.0
        }

        KPublicTransport.OccupancyIndicator {
            id: occupancyIndicator
            Layout.column: 3
            Layout.row: 1
            Layout.alignment:  Qt.AlignTop
            Layout.preferredHeight: Kirigami.Units.iconSizes.small
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
            occupancy: root.stop.maximumOccupancy
        }

        QQC2.Label {
            Layout.column: 4
            Layout.row: 0
            Layout.alignment:  Qt.AlignTop

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
            opacity: root.stop.pickupType === KPublicTransport.PickupDropoff.NotAllowed && root.stop.dropoffType === KPublicTransport.PickupDropoff.NotAllowed ? 0.5 : 1.0
        }

        QQC2.ToolButton {
            Layout.column: 5
            Layout.row: 0
            Layout.alignment:  Qt.AlignTop
            Layout.topMargin: -(height - arrivalTime.height) / 2
            Layout.bottomMargin: -(height - arrivalTime.height) / 2
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

        QQC2.Label {
            id: notesLabel
            Layout.column: 3
            Layout.row: (occupancyIndicator.visible || !root.isSingleTime) ? 2 : 1
            Layout.rowSpan: (occupancyIndicator.visible || !root.isSingleTime) ? 1 : 2
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
