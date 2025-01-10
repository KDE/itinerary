/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KirigamiDelegates
import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary

Kirigami.ScrollablePage {
    id: root
    title: i18n("Vehicle Layout")

    property alias publicTransportManager: vehicleModel.manager
    property var stopover
    property string selectedVehicleSection
    property string selectedClasses
    property string seat

    readonly property alias vehicleLayout: vehicleModel.stopover
    signal layoutUpdated()

    readonly property var selectedClassTypes: {
        var c = KPublicTransport.VehicleSection.UnknownClass;
        if (selectedClasses.match(/1/)) {
            c |= KPublicTransport.VehicleSection.FirstClass;
        }
        if (selectedClasses.match(/2/)) {
            c |= KPublicTransport.VehicleSection.SecondClass;
        }

        return c;
    }

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
            if (selectedVehicleSection) {
                offset = vehicleView.fullLength * vehicleModel.vehicle.platformPositionForSection(selectedVehicleSection);
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

    function classesLabel(cls) {
        if (cls == KPublicTransport.VehicleSection.FirstClass)
            return i18n("First class");
        if (cls == KPublicTransport.VehicleSection.SecondClass)
            return i18n("Second class");
        if (cls == (KPublicTransport.VehicleSection.FirstClass | KPublicTransport.VehicleSection.SecondClass))
            return i18n("First/second class");
        return i18n("Unknown class");
    }

    SheetDrawer {
        id: coachDrawer
        property var coach
        headerItem: Component {
            // TODO show platform section as well?
            ColumnLayout {
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Kirigami.Heading {
                    text: i18nc("train coach", "Coach %1", coachDrawer.coach.name)
                    Layout.fillWidth: true
                    elide: Qt.ElideRight
                }
                QQC2.Label {
                    id: subtitle
                    Layout.fillWidth: true
                    text: {
                        switch (coachDrawer.coach.type) {
                            case KPublicTransport.VehicleSection.SleepingCar:
                                return i18nc("train coach type", "Sleeping car");
                            case KPublicTransport.VehicleSection.CouchetteCar:
                                return i18nc("train coach type", "Couchette car");
                            case KPublicTransport.VehicleSection.RestaurantCar:
                                return i18nc("train coach type", "Restaurant car");
                            case KPublicTransport.VehicleSection.CarTransportCar:
                                return i18nc("train coach type", "Car transport car");
                            default:
                                break
                        }
                        return classesLabel(coachDrawer.coach.classes);
                    }
                    visible: subtitle.text !== ""
                }
            }
        }

        contentItem: Component {
            ColumnLayout {
                PublicTransportFeatureList {
                    model: coachDrawer.coach.sectionFeatures
                }
            }
        }
    }

    function colorMix(bg, fg, alpha)
    {
        return Qt.tint(bg, Qt.rgba(fg.r, fg.g, fg.b, alpha));
    }

    header: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            KPublicTransport.TransportNameControl {
                line: vehicleModel.stopover.route.line
            }

            QQC2.Label {
                text: i18nc("Direction of the transport mode", "To %1", vehicleModel.stopover.stopPoint.name)
                visible: departure.route.direction
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
                model: root.stopover.features
                delegate: KPublicTransport.FeatureIcon {
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
            onLinkActivated: Qt.openUrlExternally(link)

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
            PublicTransportFeatureList {
                model: root.stopover.features
            }
            QQC2.Label {
                Layout.fillWidth: true
                text: root.stopover.notes.join("<br/>")
                textFormat: Text.RichText
                wrapMode: Text.Wrap
                onLinkActivated: Qt.openUrlExternally(link)
                padding: Kirigami.Units.largeSpacing * 2
            }
        }
    }

    Item {
        id: vehicleView
        property real fullLength: vehicleModel.platform.hasAbsoluteLength ? vehicleModel.platform.length * 3.5 : 1600 // full length of the platform display
        property real sectionWidth: 48
        width: parent.width
        height: vehicleView.fullLength
        implicitHeight: height

        Repeater {
            width: parent.width
            model: vehicleModel.platform.sections
            delegate: Item {
                property var section: modelData
                width: parent.width
                y: section.begin * vehicleView.fullLength
                height: section.end * vehicleView.fullLength - y

                Kirigami.Separator {
                    visible: index == 0
                    anchors { top: parent.top; left: parent.left; right: parent.right }
                }
                QQC2.Label {
                    anchors.centerIn: parent
                    text: section.name
                }
                Kirigami.Separator {
                    anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
                }
            }
        }

        Kirigami.Icon {
            visible: vehicleModel.vehicle.direction != KPublicTransport.Vehicle.UnknownDirection
            source: {
                if (vehicleModel.vehicle.direction == KPublicTransport.Vehicle.Forward)
                    return "go-up";
                if (vehicleModel.vehicle.direction == KPublicTransport.Vehicle.Backward)
                    return "go-down"
                return "";
            }
            width: Kirigami.Units.iconSizes.small
            height: width
            x: vehicleView.sectionWidth / 2 - width / 2
            y: vehicleModel.vehicle.platformPositionBegin * vehicleView.fullLength - height - Kirigami.Units.largeSpacing
        }
        Repeater {
            id: vehicleRepeater
            width: parent.width
            model: vehicleModel
            delegate: KPublicTransport.VehicleSectionItem {
                readonly property bool isSelected: {
                    if (root.selectedVehicleSection == "") {
                        return root.selectedClassTypes & section.classes;
                    }
                    return section.name == root.selectedVehicleSection;
                }

                section: model.vehicleSection
                y: section.platformPositionBegin * vehicleView.fullLength
                height: section.platformPositionEnd * vehicleView.fullLength - y
                width: vehicleView.sectionWidth
                textColor: model.vehicleSection.disruptionEffect === KPublicTransport.Disruption.NoService ?
                        Kirigami.Theme.disabledTextColor :  Kirigami.Theme.textColor
                firstClassBackground: colorMix(Kirigami.Theme.backgroundColor, Kirigami.Theme.positiveTextColor, isSelected ? 1 : 0.25)
                secondClassBackground: colorMix(Kirigami.Theme.backgroundColor, Kirigami.Theme.focusColor, isSelected ? 1 : 0.25)
                inaccessibleBackground: colorMix(Kirigami.Theme.backgroundColor, Kirigami.Theme.disabledTextColor, isSelected ? 1 : 0.25)
                restaurantBackground: colorMix(Kirigami.Theme.backgroundColor, Kirigami.Theme.neutralTextColor, isSelected ? 1 : 0.25)

                ColumnLayout {
                    anchors.centerIn: parent
                    QQC2.Label {
                        Layout.alignment: Qt.AlignCenter
                        text: section.name
                        visible: text !== ""
                        color: model.vehicleSection.disruptionEffect === KPublicTransport.Disruption.NoService ?
                            Kirigami.Theme.disabledTextColor :  Kirigami.Theme.textColor
                    }
                    Kirigami.Icon {
                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                        Layout.preferredHeight: visible ? Kirigami.Units.iconSizes.small : 0
                        source: section.type !== KPublicTransport.vehicleSection.ControlCar ? section.iconName : ""
                        color: model.vehicleSection.disruptionEffect === KPublicTransport.Disruption.NoService ?
                            Kirigami.Theme.disabledTextColor :  Kirigami.Theme.textColor
                        isMask: true
                        visible: source ? true : false
                    }
                }

                TapHandler {
                    enabled: model.vehicleSection.sectionFeatures.length > 0 && model.vehicleSection.disruptionEffect !== KPublicTransport.Disruption.NoService
                    onTapped: {
                        coachDrawer.coach = model.vehicleSection;
                        coachDrawer.open();
                    }
                }

                ColumnLayout {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.right
                    anchors.leftMargin: Kirigami.Units.largeSpacing
                    spacing: Kirigami.Units.smallSpacing

                    RowLayout {
                        spacing: Kirigami.Units.smallSpacing
                        visible: model.vehicleSection.disruptionEffect !== KPublicTransport.Disruption.NoService
                        Repeater {
                            model: section.sectionFeatures
                            delegate: KPublicTransport.FeatureIcon {
                                feature: modelData
                                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                            }
                        }
                    }
                    QQC2.Label {
                        visible: section.classes != KPublicTransport.VehicleSection.UnknownClass
                        text: classesLabel(section.classes)
                        color: model.vehicleSection.disruptionEffect === KPublicTransport.Disruption.NoService ?
                                Kirigami.Theme.disabledTextColor :  Kirigami.Theme.textColor
                    }
                    KPublicTransport.OccupancyIndicator {
                        occupancy: model.vehicleSection.load
                        Layout.preferredHeight: Kirigami.Units.iconSizes.small
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                    }

                    TapHandler {
                        enabled: model.vehicleSection.sectionFeatures.length > 0 && model.vehicleSection.disruptionEffect !== KPublicTransport.Disruption.NoService
                        onTapped: {
                            coachDrawer.coach = model.vehicleSection;
                            coachDrawer.open();
                        }
                    }
                }
            }
        }
        Kirigami.Icon {
            visible: vehicleModel.vehicle.direction != KPublicTransport.Vehicle.UnknownDirection
            source: {
                if (vehicleModel.vehicle.direction == KPublicTransport.Vehicle.Forward)
                    return "go-up";
                if (vehicleModel.vehicle.direction == KPublicTransport.Vehicle.Backward)
                    return "go-down"
                return "";
            }
            width: Kirigami.Units.iconSizes.small
            height: width
            x: vehicleView.sectionWidth / 2 - width / 2
            y: vehicleModel.vehicle.platformPositionEnd * vehicleView.fullLength + Kirigami.Units.largeSpacing
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

        QQC2.Label {
            anchors.centerIn: parent
            parent: root
            width: vehicleView.width
            visible: vehicleModel.errorMessage === "" && !vehicleModel.loading && vehicleRepeater.count === 0
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            text: i18n("No vehicle layout information available.")
        }
    }

    Component.onCompleted: contentItem.clip = true // workaround for Android not doing this automatically
}

