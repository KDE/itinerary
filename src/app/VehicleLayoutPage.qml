/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kpublictransport 1.0 as KPublicTransport
import org.kde.kpublictransport.ui 1.0
import org.kde.itinerary 1.0

Kirigami.ScrollablePage {
    id: root
    title: i18n("Vehicle Layout")

    property alias publicTransportManager: vehicleModel.manager
    property var stopover
    property string selectedVehicleSection
    property string selectedClasses
    property string seat

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
        }
    }

    Item {
        id: vehicleView
        property real fullLength: vehicleModel.platform.hasAbsoluteLength ? vehicleModel.platform.length * 3.5 : 1600 // full length of the platform display
        property real sectionWidth: 48
        width: parent.width
        height: vehicleView.fullLength

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
                            }
                        }
                        color: Kirigami.Theme.textColor
                        isMask: true
                        visible: source ? true : false
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
                            model: section.featureList
                            QQC2.Label {
                                text: {
                                    switch (modelData) {
                                        case KPublicTransport.VehicleSection.AirConditioning: return "❄️";
                                        case KPublicTransport.VehicleSection.Restaurant: return "🍴";
                                        case KPublicTransport.VehicleSection.ToddlerArea: return "👶";
                                        case KPublicTransport.VehicleSection.WheelchairAccessible: return "♿";
                                        case KPublicTransport.VehicleSection.SilentArea: return "🔇";
                                        case KPublicTransport.VehicleSection.BikeStorage: return "🚲";
                                    }
                                }
                            }
                        }
                    }
                    QQC2.Label {
                        visible: section.classes != KPublicTransport.VehicleSection.UnknownClass
                        text: {
                            if (section.classes == KPublicTransport.VehicleSection.FirstClass)
                                return i18n("First class");
                            if (section.classes == KPublicTransport.VehicleSection.SecondClass)
                                return i18n("Second class");
                            if (section.classes == (KPublicTransport.VehicleSection.FirstClass | KPublicTransport.VehicleSection.SecondClass))
                                return i18n("First/second class");
                            return i18n("Unknown class");
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
}

