/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.7 as Kirigami
import org.kde.kpublictransport 1.0 as PT
import org.kde.kosmindoormap 1.0
import org.kde.itinerary 1.0
import org.kde.kosmindoormap.kpublictransport 1.0

Kirigami.Page {
    id: root
    title: {
        if (map.mapLoader.isLoading || map.hasError)
            return placeName;
        if (map.floorLevels.hasName(map.view.floorLevel))
            return map.floorLevels.name(map.view.floorLevel);
        return i18n("Floor %1", map.floorLevels.name(map.view.floorLevel));
    }

    property point coordinate
    property string placeName
    property alias map: map

    property string arrivalGateName
    property string arrivalPlatformName
    property string departureGateName
    property string departurePlatformName

    property string region
    property string timeZone
    property date beginTime
    property date endTime

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    // prevent swipe to the right changing pages, we want to pan the map instead
    // TODO in theory we could make this conditional to having panned the map all the way to the right
    Kirigami.ColumnView.preventStealing: true

    actions {
        left: Kirigami.Action {
            iconName: "go-down-symbolic"
            enabled: map.floorLevels.hasFloorLevelBelow(map.view.floorLevel)
            onTriggered: map.view.floorLevel = map.floorLevels.floorLevelBelow(map.view.floorLevel)
            visible: map.floorLevels.hasFloorLevels
        }
        right: Kirigami.Action {
            iconName: "go-up-symbolic"
            enabled: map.floorLevels.hasFloorLevelAbove(map.view.floorLevel)
            onTriggered: map.view.floorLevel = map.floorLevels.floorLevelAbove(map.view.floorLevel)
            visible: map.floorLevels.hasFloorLevels
        }
    }
    contextualActions: [
        Kirigami.Action {
            id: zoomInAction
            text: i18n("Zoom In")
            iconName: "zoom-in-symbolic"
            onTriggered: map.view.setZoomLevel(map.view.zoomLevel + 1, Qt.point(map.width / 2.0, map.height/ 2.0));
            enabled: map.view.zoomLevel < 21
        },
        Kirigami.Action {
            id: zoomOutAction
            text: i18n("Zoom Out")
            iconName: "zoom-out-symbolic"
            onTriggered: map.view.setZoomLevel(map.view.zoomLevel - 1, Qt.point(map.width / 2.0, map.height/ 2.0));
            enabled: map.view.zoomLevel > 14
        },
        Kirigami.Action {
            id: platformAction
            text: i18n("Find Platform")
            onTriggered: platformSheet.sheetOpen = true
            visible: !platformModel.isEmpty
        },
        Kirigami.Action {
            id: gateAction
            text: i18n("Find Gate")
            onTriggered: gateSheet.sheetOpen = true
            visible: !gateModel.isEmpty
        },
        Kirigami.Action { separator: true },
        Kirigami.Action {
            id: rentalVehicleAction
            text: i18n("Show Rental Vehicles")
            checkable: true
            enabled: !map.mapLoader.isLoading
            onTriggered: queryRentalVehicles();
        },
        Kirigami.Action {
            id: lightStyleAction
            text: "Light Style"
            onTriggered: map.styleSheet = "breeze-light"
            visible: Settings.developmentMode
        },
        Kirigami.Action {
            id: darkStyleAction
            text: "Dark Style"
            onTriggered: map.styleSheet = "breeze-dark"
            visible: Settings.developmentMode
        },
        Kirigami.Action {
            id: diagnosticStyleAction
            text: "Diagnostic Style"
            onTriggered: map.styleSheet = "diagnostic"
            visible: Settings.developmentMode
        }
    ]

    OSMElementInformationModel {
        id: infoModel
        debug: Settings.developmentMode
    }

    IndoorMapInfoSheet {
        id: elementDetailsSheet
        model: infoModel
        mapData: map.mapData
    }

    PlatformModel {
        id: platformModel
        mapData: map.mapData

        onPlatformIndexChanged: {
            if (platformModel.departurePlatformRow >= 0) {
                var idx = platformModel.index(platformModel.departurePlatformRow, 0);
                map.view.floorLevel = platformModel.data(idx, PlatformModel.LevelRole)
                map.view.centerOnGeoCoordinate(platformModel.data(idx, PlatformModel.CoordinateRole));
                map.view.setZoomLevel(19, Qt.point(map.width / 2.0, map.height/ 2.0));
            }
        }
    }

    IndoorMapPlatformSheet {
        id: platformSheet
        model: platformModel
    }

    GateModel {
        id: gateModel
        mapData: map.mapData

        onGateIndexChanged: {
            if (gateModel.departureGateRow >= 0) {
                var idx = gateModel.index(gateModel.departureGateRow, 0);
                map.view.floorLevel = gateModel.data(idx, GateModel.LevelRole)
                map.view.centerOnGeoCoordinate(gateModel.data(idx, GateModel.CoordinateRole));
                map.view.setZoomLevel(18, Qt.point(map.width / 2.0, map.height/ 2.0));
            }
        }
    }

    IndoorMapGateSheet {
        id: gateSheet
        model: gateModel
    }

    FloorLevelChangeModel {
        id: floorLevelChangeModel
        currentFloorLevel: map.view.floorLevel
        floorLevelModel: map.floorLevels
    }

    IndoorMapElevatorSheet {
        id: elevatorSheet
        model: floorLevelChangeModel
    }

    LocationQueryOverlayProxyModel {
        id: locationModel
        sourceModel: PT.LocationQueryModel {
            id: locationQuery
            manager: LiveDataManager.publicTransportManager
        }
        mapData: map.mapData
    }

    function queryRentalVehicles()
    {
        if (rentalVehicleAction.checked) {
            locationQuery.request.latitude = map.mapData.center.y;
            locationQuery.request.longitude = map.mapData.center.x;
            locationQuery.request.maximumDistance = map.mapData.radius;
            locationQuery.request.types = PT.Location.RentedVehicleStation;
        } else {
            locationQuery.clear();
        }
    }

    IndoorMap {
        id: map
        anchors.fill: parent
        overlaySources: [ gateModel, platformModel, locationModel ]

        IndoorMapScale {
            map: map
            anchors.left: map.left
            anchors.top: map.top
            width: 0.3 * map.width
        }

        IndoorMapAttributionLabel {
            anchors.right: map.right
            anchors.bottom: map.bottom
        }

        onElementPicked: {
            floorLevelChangeModel.element = element;
            if (floorLevelChangeModel.hasSingleLevelChange) {
                showPassiveNotification(i18n("Switched to floor %1", floorLevelChangeModel.destinationLevelName), "short");
                map.view.floorLevel = floorLevelChangeModel.destinationLevel;
                return;
            } else if (floorLevelChangeModel.hasMultipleLevelChanges) {
                elevatorSheet.sheetOpen = true;
                return;
            }

            infoModel.element = element;
            if (infoModel.name != "" || infoModel.debug) {
                elementDetailsSheet.sheetOpen = true;
            }
        }
    }

    onCoordinateChanged: map.mapLoader.loadForCoordinate(coordinate.y, coordinate.x);

    Connections {
        target: map.mapLoader
        function onDone() {
            platformModel.setArrivalPlatform(root.arrivalPlatformName, Platform.Rail);
            platformModel.setDeparturePlatform(root.departurePlatformName, Platform.Rail);
            gateModel.setArrivalGate(root.arrivalGateName);
            gateModel.setDepartureGate(root.departureGateName);
            map.region = root.region;
            map.timeZone = root.timeZone;
            map.view.beginTime = root.beginTime;
            map.view.endTime = root.endTime;
            queryRentalVehicles();
        }
    }
}
