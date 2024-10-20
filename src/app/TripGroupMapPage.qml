/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtLocation as QtLocation
import QtPositioning as QtPositioning
import org.kde.kirigami as Kirigami
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
            QtLocation.MapQuickItem {
                coordinate {
                    latitude: modelData.journeySection.departure.stopPoint.latitude
                    longitude: modelData.journeySection.departure.stopPoint.longitude
                }
                anchorPoint.x: sourceItem.width / 2
                anchorPoint.y: sourceItem.height / 2
                sourceItem: Rectangle {
                    width: 15
                    height: 15
                    radius: height / 2
                    border.width: 2
                    border.color: modelData.color
                    color: modelData.textColor
                    MouseArea {
                        anchors.fill: parent
                        scale: 2
                        onClicked: {
/*                            sheetDrawer.open()
                            sheetDrawer.isArrival = false
                            sheetDrawer.isDeparture = true
                            sheetDrawer.stop = journeySection.departure*/
                        }
                    }
                }
            }
        }

        // intermediate stops
        Repeater {
            model: mapModel.journeySections
            Repeater {
                property var pathEntry: modelData
                model: modelData.journeySection.intermediateStops
                QtLocation.MapQuickItem {
                    coordinate {
                        latitude: modelData.stopPoint.latitude
                        longitude: modelData.stopPoint.longitude
                    }
                    anchorPoint.x: sourceItem.width / 2
                    anchorPoint.y: sourceItem.height / 2
                    sourceItem: Rectangle {
                        height: pathEntry.width
                        width: height
                        radius: height / 2
                        opacity: 0.5
                        color: pathEntry.textColor
                        border.width: 1
                        border.color: pathEntry.color
                        MouseArea {
                            anchors.fill: parent
                            scale: 3
/*                            onClicked: {
                                sheetDrawer.open()
                                sheetDrawer.isArrival = false
                                sheetDrawer.isDeparture = false
                                sheetDrawer.stop = model.stopover
                            }*/
                        }
                    }
                }
            }
        }

        // path maneauvers
        // TODO

        // arrival nodes
        Repeater {
            model: mapModel.journeySections
            QtLocation.MapQuickItem {
                coordinate {
                    latitude: modelData.journeySection.arrival.stopPoint.latitude
                    longitude: modelData.journeySection.arrival.stopPoint.longitude
                }
                anchorPoint.x: sourceItem.width / 2
                anchorPoint.y: sourceItem.height / 2
                sourceItem: Rectangle {
                    width: 15
                    height: 15
                    radius: height / 2
                    border.width: 2
                    border.color: modelData.color
                    color: modelData.textColor
                    MouseArea {
                        anchors.fill: parent
                        scale: 2
                        onClicked: {
/*                            sheetDrawer.open()
                            sheetDrawer.isArrival = false
                            sheetDrawer.isDeparture = true
                            sheetDrawer.stop = journeySection.arrival*/
                        }
                    }
                }
            }
        }

        // points
        Repeater {
            model: mapModel.points
            QtLocation.MapQuickItem {
                coordinate {
                    latitude: modelData.location.latitude
                    longitude: modelData.location.longitude
                }
                anchorPoint.x: sourceItem.width / 2
                anchorPoint.y: sourceItem.height
                sourceItem: Kirigami.Icon {
                    id: mainIcon
                    width: height
                    height: Kirigami.Units.iconSizes.huge
                    source: "map-symbolic"
                    isMask: true
                    color: modelData.color

                    Rectangle {
                        anchors.centerIn: parent
                        anchors.verticalCenterOffset: -parent.height / 8
                        width: height
                        height: parent.height / 3 + 1
                        radius: height / 2
                        color: "white" // TODO
                    }
                    Kirigami.Icon {
                        anchors.centerIn: parent
                        anchors.verticalCenterOffset: -parent.height / 8
                        width: height
                        height: parent.height / 3 + 1
                        source: modelData.iconName
                        isMask: true
                        color: parent.color
                    }
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
}
