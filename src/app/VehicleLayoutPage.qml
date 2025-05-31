/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary

Kirigami.ScrollablePage {
    id: root
    title: i18n("Vehicle Layout")

    property alias publicTransportManager: vehicleModel.manager
    property KPublicTransport.stopover stopover
    property string selectedVehicleSection
    property string selectedClasses
    property string seat

    readonly property alias vehicleLayout: vehicleModel.stopover
    signal layoutUpdated()

    readonly property int selectedClassTypes: KPublicTransport.ClassUtil.fromString(root.selectedClasses)

    onStopoverChanged: vehicleModel.request.stopover = root.stopover;

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor

        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
    }

    topPadding: 0

    KPublicTransport.VehicleLayoutQueryModel {
        id: vehicleModel

        onContentChanged: {
            var offset = 0;
            if (root.selectedVehicleSection) {
                offset = vehicleView.fullLength * vehicleModel.vehicle.platformPositionForSection(root.selectedVehicleSection);
                offset -= root.flickable.height / 2; // place in center
            } else {
                offset = vehicleView.fullLength * vehicleModel.vehicle.platformPositionBegin;
                offset -= Kirigami.Units.iconSizes.small + Kirigami.Units.largeSpacing; // direction indicator
            }

            offset = Math.max(offset, 0);
            root.flickable.contentY = offset;

            if (vehicleModel.rowCount() > 0)
                root.layoutUpdated();
        }
    }

    SheetDrawer {
        id: coachDrawer
        property KPublicTransport.vehicleSection coach
        headerItem: Component {
            // TODO show platform section as well?
            ColumnLayout {
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Kirigami.Heading {
                    text: i18nc("train coach", "Coach %1", coachDrawer.coach.name)
                    Layout.fillWidth: true
                    elide: Qt.ElideRight
                }
                QQC2.Label {Articles de cortesia
                Repeater {
                    model: coachDrawer.coach.sectionFeatures
                    delegate: KPublicTransport.FeatureDelegate {
                        Layout.leftMargin: Kirigami.Units.largeSpacing
                        required property KPublicTransport.feature modelData
                        feature: modelData
                    }
                }
                KPublicTransport.OccupancyDelegate {
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                    occupancy: coachDrawer.coach.load
                    visible: occupancy != KPublicTransport.Load.Unknown
                }
            }
        }
    }

    header: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            KPublicTransport.TransportNameControl {
                line: vehicleModel.stopover.route.line
            }

            QQC2.Label {
                text: i18nc("Direction of the transport mode", "To %1", vehicleModel.stopover.route.direction)
                visible: vehicleModel.stopover.route.direction
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
        }

        QQC2.Label {

            wrapMode: Text.Wrap
            text: if (vehicleModel.stopover.scheduledDepartureTime > 0) {
                i18n("Departure: %1", Localizer.formatDateTime(vehicleModel.stopover, "scheduledDepartureTime"))
            } else if (vehicleModel.stopover.scheduledArrivalTime > 0) {
                i18n("Arrival: %1", Localizer.formatDateTime(vehicleModel.stopover, "scheduledArrivalTime"))
            }

            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
        }

        QQC2.Label {
            property string platformName: {
                if (vehicleModel.platform.name)
                    return vehicleModel.platform.name;
                if (vehicleModel.stopover.expectedPlatform)
                    return vehicleModel.stopover.expectedPlatform;
                if (vehicleModel.stopover.scheduledPlatform)
                    return vehicleModel.stopover.scheduledPlatform;
                return "-";
            }

            wrapMode: Text.Wrap
            text: i18n("Platform: %1", platformName)

            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
        }

        QQC2.Label {
            wrapMode: Text.Wrap
            text: i18n("Coach: %1 Seat: %2", (root.selectedVehicleSection ? root.selectedVehicleSection : "-"), (root.seat ? root.seat : "-"))
            visible: root.selectedVehicleSection !== "" || root.seat !== ""

            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Repeater {
                model: root.stopover.vehicleLayout.combinedFeatures
                delegate: KPublicTransport.FeatureIcon {
                    required property KPublicTransport.feature modelData
                    feature: modelData
                    Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    Layout.preferredWidth: Kirigami.Units.iconSizes.small
                }
            }

            TapHandler {
                onTapped: vehicleSheet.open()
            }

            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
        }

        QQC2.Label {
            id: notesLabel

            text: root.stopover.notes.join("<br/>")
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            // Doesn't work with RichText.
            elide: Text.ElideRight
            maximumLineCount: 5
            clip: true
            visible: root.stopover.notes.length > 0
            font.italic: true
            onLinkActivated: (link) => { Qt.openUrlExternally(link); }

            Layout.maximumHeight: Kirigami.Units.gridUnit * maximumLineCount
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
        }

        Kirigami.LinkButton {
            text: i18nc("@action:button", "Show More…")
            visible: notesLabel.implicitHeight > notesLabel.height
            onClicked: vehicleSheet.open();

            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }
    }

    data: SheetDrawer {
        id: vehicleSheet

        contentItem: ColumnLayout {
            Kirigami.Heading {
                text: i18n("Information")
                level: 4
                Layout.leftMargin: Kirigami.Units.largeSpacing
                visible: root.stopover.notes.length > 0
            }
            QQC2.Label {
                Layout.fillWidth: true
                text: root.stopover.notes.join("<br/>")
                textFormat: Text.RichText
                wrapMode: Text.Wrap
                font.italic: true
                onLinkActivated: (link) => { Qt.openUrlExternally(link); }
                leftPadding: Kirigami.Units.largeSpacing
                rightPadding: Kirigami.Units.largeSpacing
                visible: root.stopover.notes.length > 0
            }
            Kirigami.Heading {
                text: i18n("Occupancy")
                level: 4
                Layout.leftMargin: Kirigami.Units.largeSpacing
                visible: root.stopover.aggregatedOccupancy.length > 0
            }
            Repeater {
                model: root.stopover.aggregatedOccupancy
                delegate: KPublicTransport.OccupancyDelegate {
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                    required property KPublicTransport.loadInfo modelData
                    occupancyInfo: modelData
                }
            }
            Kirigami.Heading {
                text: i18n("Amenities")
                level: 4
                Layout.leftMargin: Kirigami.Units.largeSpacing
                visible: root.stopover.vehicleLayout.combinedFeatures.length > 0
            }
            Repeater {
                model: root.stopover.vehicleLayout.combinedFeatures
                delegate: KPublicTransport.FeatureDelegate {
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                    required property KPublicTransport.feature modelData
                    feature: modelData
                }
            }
        }
    }

    KPublicTransport.VehicleLayoutView {
        id: vehicleView
        width: parent.width

        model: vehicleModel
        highlightedVehicleSection: root.selectedVehicleSection
        higlightedClassTypes: root.selectedClassTypes

        onVehicleSectionTapped: (section) => {
            coachDrawer.coach = section;
            coachDrawer.open();
        }
    }

    QQC2.BusyIndicator {
        anchors.centerIn: parent
        parent: root
        running: vehicleModel.loading
    }

    QQC2.Label {
        anchors.centerIn: parent
        parent: root
        width: vehicleView.width
        text: vehicleModel.errorMessage
        color: Kirigami.Theme.negativeTextColor
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
    }

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        parent: root
        width: vehicleView.width
        visible: vehicleModel.errorMessage === "" && !vehicleModel.loading && vehicleModel.vehicle.sections.length === 0
        text: i18n("No vehicle layout information available.")
    }

    Component.onCompleted: contentItem.clip = true // workaround for Android not doing this automatically
}

