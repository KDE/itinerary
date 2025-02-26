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
                autoFadeIn: false
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
                visible: modelData.showStart
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
            model: mapModel.journeySections.filter((jny) => { return jny.journeySection.mode == KPublicTransport.JourneySection.Walking || jny.journeySection.mode == KPublicTransport.JourneySection.RentedVehicle || jny.journeySection.mode == KPublicTransport.JourneySection.IndividualTransport || jny.journeySection.mode == KPublicTransport.JourneySection.Transfer; })
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
                visible: modelData.showEnd
                onClicked: {
                    stopInfoDrawer.open()
                    stopInfoDrawer.isArrival = true
                    stopInfoDrawer.isDeparture = false
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

    MapStopoverInfoSheetDrawer {
        id: stopInfoDrawer
        anchors.fill: parent
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
