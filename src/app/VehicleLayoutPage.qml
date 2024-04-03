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
import org.kde.kpublictransport.ui
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

    header: Item {
        height: childrenRect.height + 2 * Kirigami.Units.gridUnit
        GridLayout {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Kirigami.Units.gridUnit

            columns: 2
            columnSpacing: Kirigami.Units.largeSpacing
            rows: 4
            rowSpacing: 0

            Kirigami.Icon {
                Layout.rowSpan: 4
                id: icon
                source: PublicTransport.lineIcon(vehicleModel.stopover.route.line)
                width: height
                height: Kirigami.Units.iconSizes.large
                isMask: !vehicleModel.stopover.route.line.hasLogo && !vehicleModel.stopover.route.line.hasModeLogo
            }

            QQC2.Label {
                Layout.row: 0
                Layout.column: 1
                Layout.fillWidth: true
                text: "<b>" + vehicleModel.stopover.route.line.modeString + " " + vehicleModel.stopover.route.line.name + "</b> ("
                            + vehicleModel.stopover.stopPoint.name + ")"
            }

            QQC2.Label {
                Layout.row: 1
                Layout.column: 1
                Layout.columnSpan: 2
                text: if (vehicleModel.stopover.scheduledDepartureTime > 0) {
                    i18n("Departure: %1", Localizer.formatDateTime(vehicleModel.stopover, "scheduledDepartureTime"))
                } else if (vehicleModel.stopover.scheduledArrivalTime > 0) {
                    i18n("Arrival: %1", Localizer.formatDateTime(vehicleModel.stopover, "scheduledArrivalTime"))
                }
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

                Layout.row: 2
                Layout.column: 1
                Layout.columnSpan: 2
                text: i18n("Platform: %1", platformName)
            }
            QQC2.Label {
                Layout.row: 3
                Layout.column: 1
                Layout.columnSpan: 2
                text: i18n("Coach: %1 Seat: %2", (selectedVehicleSection ? selectedVehicleSection : "-"), (seat ? seat : "-"))
            }

            RowLayout {
                Layout.row: 4
                Layout.column: 1
                Layout.columnSpan: 2
                spacing: Kirigami.Units.smallSpacing
                Repeater {
                    model: root.stopover.features
                    delegate: PublicTransportFeatureIcon {
                        feature: modelData
                    }
                }

                TapHandler {
                    onTapped: vehicleSheet.open()
                }
            }

            QQC2.Label {
                id: notesLabel
                Layout.row: 5
                Layout.column: 1
                Layout.columnSpan: 2
                Layout.fillWidth: true
                text: root.stopover.notes.join("<br/>")
                textFormat: Text.RichText
                wrapMode: Text.Wrap
                verticalAlignment: Text.AlignTop
                // Doesn't work with RichText.
                elide: Text.ElideRight
                maximumLineCount: 5
                Layout.maximumHeight: Kirigami.Units.gridUnit * maximumLineCount
                clip: true
                visible: root.stopover.notes.length > 0
                font.italic: true
                onLinkActivated: Qt.openUrlExternally(link)
            }
            Kirigami.LinkButton {
                Layout.row: 6
                Layout.column: 1
                Layout.columnSpan: 2
                text: i18nc("@action:button", "Show More…")
                visible: notesLabel.implicitHeight > notesLabel.height
                onClicked: vehicleSheet.open();
            }
        }

        SheetDrawer {
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
            delegate: VehicleSectionItem {
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
                textColor: Kirigami.Theme.textColor
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
                    }
                    Kirigami.Icon {
                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                        Layout.preferredHeight: visible ? Kirigami.Units.iconSizes.small : 0
                        source: {
                            switch (section.type) {
                                case KPublicTransport.VehicleSection.PassengerCar: return "qrc:///images/seat.svg"
                                case KPublicTransport.VehicleSection.SleepingCar: return "qrc:///images/sleepingcar.svg"
                                case KPublicTransport.VehicleSection.CouchetteCar: return "qrc:///images/couchettecar.svg"
                                case KPublicTransport.VehicleSection.RestaurantCar: return "qrc:///images/foodestablishment.svg"
                                case KPublicTransport.VehicleSection.CarTransportCar: return "qrc:///images/car.svg"
                            }
                        }
                        color: Kirigami.Theme.textColor
                        isMask: true
                        visible: source ? true : false
                    }
                }

                TapHandler {
                    enabled: model.vehicleSection.sectionFeatures.length > 0
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
                        Repeater {
                            model: section.sectionFeatures
                            delegate: PublicTransportFeatureIcon {
                                feature: modelData
                            }
                        }
                    }
                    QQC2.Label {
                        visible: section.classes != KPublicTransport.VehicleSection.UnknownClass
                        text: classesLabel(section.classes)
                    }

                    TapHandler {
                        enabled: model.vehicleSection.sectionFeatures.length > 0
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

