/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtLocation as QtLocation
import QtPositioning as QtPositioning
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary

Kirigami.Page {
    id: root

    property alias tripGroupId: mapModel.tripGroupId

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    title: TripGroupManager.tripGroup(root.tripGroupId).name

    // prevent swipe to the right changing pages, we want to pan the map instead
    Kirigami.ColumnView.preventStealing: true

    TripGroupMapModel {
        id: mapModel
        tripGroupId: root.tripGroupId
        tripGroupManager: TripGroupManager
        liveDataManager: LiveDataManager
        transferManager: TransferManager
    }

    MapView {
        id: map
        anchors.fill: parent

        // paths
        Repeater {
            model: mapModel.journeySections
            QtLocation.MapPolyline {
                line.width: modelData.width
                line.color: modelData.color
                path: KPublicTransport.MapUtils.polyline(modelData.journeySection);
                referenceSurface: modelData.journeySection.route.line.mode == KPublicTransport.Line.Air ? QtLocation.QtLocation.ReferenceSurface.Globe : QtLocation.QtLocation.ReferenceSurface.Map
            }
        }

        // departure nodes
        Repeater {
            model: mapModel.journeySections
            MapCircle {
                coordinate {
                    latitude: modelData.journeySection.departure.stopPoint.latitude
                    longitude: modelData.journeySection.departure.stopPoint.longitude
                }
                color: modelData.color
                textColor: modelData.textColor
                size: 15
                onClicked: {
                    stopInfoDrawer.open();
                    stopInfoDrawer.isArrival = false;
                    stopInfoDrawer.isDeparture = true;
                    stopInfoDrawer.stop = modelData.journeySection.departure;
                }
            }
        }

        // intermediate stops
        Repeater {
            model: mapModel.journeySections
            Repeater {
                property var pathEntry: modelData
                model: modelData.journeySection.intermediateStops
                MapCircle {
                    coordinate {
                        latitude: modelData.stopPoint.latitude
                        longitude: modelData.stopPoint.longitude
                    }
                    color: pathEntry.color
                    size: pathEntry.width
                    borderWidth: 1
                    textColor: Qt.alpha(pathEntry.textColor, 0.5)
                    onClicked: {
                        stopInfoDrawer.open();
                        stopInfoDrawer.isArrival = false;
                        stopInfoDrawer.isDeparture = false;
                        stopInfoDrawer.stop = modelData
                    }
                }
            }
        }

        // path maneauvers
        Repeater {
            model: mapModel.journeySections.filter((jny) => { return jny.journeySection.mode == KPublicTransport.journeySection.Walking; })
            Repeater {
                property var jnySection: modelData
                model: modelData.journeySection.path.sections
                MapCircle {
                    coordinate {
                        latitude: modelData.startPoint.y
                        longitude: modelData.startPoint.x
                    }
                    color: jnySection.color
                    textColor: jnySection.textColor
                    visible: map.zoomLevel > 17
                    size: map.zoomLevel > 19 ? 22 : 16
                    rotation: modelData.maneuver == KPublicTransport.PathSection.Move ? modelData.direction : 0
                    iconName: modelData.maneuver == KPublicTransport.PathSection.Move ? "go-up" : modelData.iconName
                }
            }
        }

        // arrival nodes
        Repeater {
            model: mapModel.journeySections
            MapCircle {
                coordinate {
                    latitude: modelData.journeySection.arrival.stopPoint.latitude
                    longitude: modelData.journeySection.arrival.stopPoint.longitude
                }
                color: modelData.color
                textColor: modelData.textColor
                size: 15
                onClicked: {
                    stopInfoDrawer.open()
                    stopInfoDrawer.isArrival = false
                    stopInfoDrawer.isDeparture = true
                    stopInfoDrawer.stop = modelData.journeySection.arrival
                }
            }
        }

        // points
        Repeater {
            model: mapModel.points
            MapPin {
                coordinate {
                    latitude: modelData.location.latitude
                    longitude: modelData.location.longitude
                }
                color: modelData.color
                textColor: modelData.textColor
                iconName: modelData.iconName
                onClicked: {
                    locationInfoDrawer.open();
                    locationInfoDrawer.location = modelData.location;
                }
            }
        }

        function centerOnJourney() {
            map.center = KPublicTransport.MapUtils.center(mapModel.boundingBox);
            map.zoomLevel = KPublicTransport.MapUtils.zoomLevel(mapModel.boundingBox, map.width, map.height);
        }

        onWidthChanged: centerOnJourney();
        onHeightChanged: centerOnJourney();
        Component.onCompleted: centerOnJourney();
    }

    // TODO share with JourneySectionPage?
    SheetDrawer {
        id: stopInfoDrawer
        property KPublicTransport.stopover stop
        property bool isDeparture: false
        property bool isArrival: false
        anchors.fill: parent
        headerItem: Component {
            RowLayout {
                id: headerLayout
                Kirigami.Heading {
                    text: stopInfoDrawer.stop.stopPoint.name
                    Layout.fillWidth: true
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                    elide: Qt.ElideRight
                }

                QQC2.Label {
                    id: departureTime
                    Layout.rightMargin: delayLabel.visible ? 0:Kirigami.Units.largeSpacing
                    text: Localizer.formatTime(stopInfoDrawer.stop, "scheduledDepartureTime")
                    font.strikeout: stopInfoDrawer.stop.disruptionEffect === KPublicTransport.Disruption.NoService
                }
                QQC2.Label {
                    id: delayLabel
                    Layout.rightMargin: Kirigami.Units.largeSpacing
                    text: (stopInfoDrawer.stop.departureDelay >= 0 ? "+" : "") + stopInfoDrawer.stop.departureDelay
                    color: stopInfoDrawer.stop.departureDelay > 1 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                    visible: departureTime.visible && stopInfoDrawer.stop.hasExpectedDepartureTime && stopInfoDrawer.stop.disruptionEffect !== KPublicTransport.Disruption.NoService
                }
            }
        }

        contentItem: Component{
            ColumnLayout {
                id: contentLayout
                spacing:0
                width: stopInfoDrawer.width
                FormCard.FormTextDelegate {
                    Layout.fillWidth: true
                    id: platformDelegate
                    text: i18n("Platform:")
                    description: stopInfoDrawer.stop.hasExpectedPlatform ? stopInfoDrawer.stop.expectedPlatform : stopInfoDrawer.stop.scheduledPlatform;
                    visible: description
                }
                FormCard.AbstractFormDelegate {
                    Layout.fillWidth: true
                    visible: PublicTransport.maximumOccupancy(stopInfoDrawer.stop.loadInformation) != KPublicTransport.Load.Unknown
                        contentItem: ColumnLayout {
                            spacing: Kirigami.Units.mediumSpacing

                            QQC2.Label {
                                text: i18n("Occupancy:")
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                                Accessible.ignored: true
                            }
                            KPublicTransport.OccupancyIndicator {
                                occupancy: PublicTransport.maximumOccupancy(stopInfoDrawer.stop.loadInformation)
                                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                            }
                        }

                    background: Item{}
                }
                Item{Layout.fillHeight: true}
                FormCard.FormDelegateSeparator {}
                FormCard.FormButtonDelegate {
                    Layout.fillWidth: true
                    text: i18n("Show indoor map")
                    icon.name: "map-symbolic"
                    onClicked: {
                        stopInfoDrawer.close();
                        const args = {
                            coordinate: Qt.point(stopInfoDrawer.stop.stopPoint.longitude, stopInfoDrawer.stop.stopPoint.latitude),
                            placeName: stopInfoDrawer.stop.stopPoint.name
                        };
                        if (!stopInfoDrawer.isDeparture) {
                            args.arrivalPlatformName = stopInfoDrawer.stop.hasExpectedPlatform ? stopInfoDrawer.stop.expectedPlatform : stopInfoDrawer.stop.scheduledPlatform;
                            args.arrivalPlatformMode = PublicTransport.lineModeToPlatformMode(stopInfoDrawer.stop.route.line.mode);
                            args.arrivalPlatformIfopt = stopInfoDrawer.stop.stopPoint.identifier("ifopt");
                        }
                        if (!stopInfoDrawer.isArrival) {
                            args.departurePlatformName = stopInfoDrawer.stop.hasExpectedPlatform ? stopInfoDrawer.stop.expectedPlatform : stopInfoDrawer.stop.scheduledPlatform;
                            args.departurePlatformMode = PublicTransport.lineModeToPlatformMode(stopInfoDrawer.stop.route.line.mode);
                            args.departurePlatformIfopt = stopInfoDrawer.stop.stopPoint.identifier("ifopt");
                        }

                        // ensure the map page ends up on top
                        if (applicationWindow().pageStack.layers.depth < 2)
                            applicationWindow().pageStack.push(indoorMapPage, args);
                        else
                            applicationWindow().pageStack.layers.push(indoorMapPage, args);
                    }
                }
            }
        }
    }

    SheetDrawer {
        id: locationInfoDrawer
        property KPublicTransport.location location
        anchors.fill: parent
        headerItem: Component {
            Kirigami.Heading {
                text: locationInfoDrawer.location.name
                elide: Qt.ElideRight
            }
        }

        contentItem: Component{
            ColumnLayout {
                spacing:0
                width: locationInfoDrawer.width
                FormCard.FormTextDelegate {
                    Layout.fillWidth: true
                    id: platformDelegate
                    text: i18n("Address:")
                    description: Localizer.formatAddress(locationInfoDrawer.location)
                    visible: description
                }
            }
        }
    }
}
