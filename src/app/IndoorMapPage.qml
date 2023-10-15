/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kirigamiaddons.components 1.0 as Components
import org.kde.solidextras 1.0 as Solid
import org.kde.kpublictransport 1.0 as PT
import org.kde.kosmindoormap 1.0
import org.kde.itinerary 1.0
import org.kde.kosmindoormap.kpublictransport 1.0
import org.kde.osm.editorcontroller 1.0

Kirigami.Page {
    id: root
    title: {
        if (map.mapLoader.isLoading || map.hasError || map.floorLevels.rowCount() == 0)
            return placeName;
        if (map.floorLevels.hasName(map.view.floorLevel))
            return map.floorLevels.name(map.view.floorLevel);
        return i18n("Floor %1", map.floorLevels.name(map.view.floorLevel));
    }

    property point coordinate
    property string placeName
    property alias map: map

    property string arrivalGateName
    property alias arrivalPlatformName: platformModel.arrivalPlatform.name
    property alias arrivalPlatformMode: platformModel.arrivalPlatform.mode
    property alias arrivalPlatformIfopt: platformModel.arrivalPlatform.ifopt

    property string departureGateName
    property alias departurePlatformName: platformModel.departurePlatform.name
    property alias departurePlatformMode: platformModel.departurePlatform.mode
    property alias departurePlatformIfopt: platformModel.departurePlatform.ifopt

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

    contextualActions: [
        Kirigami.Action {
            id: zoomInAction
            text: i18n("Zoom In")
            icon.name: "zoom-in-symbolic"
            onTriggered: map.view.setZoomLevel(map.view.zoomLevel + 1, Qt.point(map.width / 2.0, map.height/ 2.0));
            enabled: map.view.zoomLevel < 21
        },
        Kirigami.Action {
            id: zoomOutAction
            text: i18n("Zoom Out")
            icon.name: "zoom-out-symbolic"
            onTriggered: map.view.setZoomLevel(map.view.zoomLevel - 1, Qt.point(map.width / 2.0, map.height/ 2.0));
            enabled: map.view.zoomLevel > 14
        },
        Kirigami.Action {
            id: platformAction
            icon.name: "search"
            text: i18n("Find Platform")
            onTriggered: platformSheet.open()
            visible: !platformModel.isEmpty
        },
        Kirigami.Action {
            id: gateAction
            icon.name: "search"
            text: i18n("Find Gate")
            onTriggered: gateSheet.open()
            visible: !gateModel.isEmpty
        },
        Kirigami.Action {
            id: amenityAction
            icon.name: "search"
            text: i18n("Find Amenity")
            onTriggered: amenitySheet.open()
        },
        Kirigami.Action { separator: true },
        Kirigami.Action {
            id: equipmentAction
            text: i18n("Show Elevator Status")
            checkable: true
            enabled: !map.mapLoader.isLoading && Solid.NetworkStatus.connectivity != Solid.NetworkStatus.No
            icon.color: Solid.NetworkStatus.metered != Solid.NetworkStatus.No ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.textColor
            onTriggered: queryLiveLocationData();
        },
        Kirigami.Action {
            id: rentalVehicleAction
            icon.name: "car"
            text: i18n("Show Rental Vehicles")
            checkable: true
            enabled: !map.mapLoader.isLoading && Solid.NetworkStatus.connectivity != Solid.NetworkStatus.No
            icon.color: Solid.NetworkStatus.metered != Solid.NetworkStatus.No ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.textColor
            onTriggered: queryLiveLocationData();
        },
        Kirigami.Action { separator: true },
        Kirigami.Action {
            text: i18n("Open Map");
            icon.name: "map-globe"
            onTriggered: NavigationController.showOnMap(map.mapData.center.y, map.mapData.center.x, 18);
        },
        Kirigami.Action {
            icon.name: "kaccess"
            text: i18n("Open wheelmap.org");
            onTriggered: NavigationController.showOnWheelmap(map.mapData.center.y, map.mapData.center.x);
        },

        Kirigami.Action {
            separator: true
            visible: Settings.osmContributorMode || Settings.developmentMode
        },
        Kirigami.Action {
            text: i18n("Edit with iD")
            icon.name: "document-edit"
            visible: Settings.osmContributorMode
            onTriggered: Settings.osmContributorMode && EditorController.editBoundingBox(root.map.view.mapSceneToGeo(root.map.view.viewport), Editor.ID)
        },
        Kirigami.Action {
            text: i18n("Edit with JOSM")
            icon.name: "org.openstreetmap.josm"
            visible: Settings.osmContributorMode && EditorController.hasEditor(Editor.JOSM)
            onTriggered: EditorController.editBoundingBox(root.map.view.mapSceneToGeo(root.map.view.viewport), Editor.JOSM)
        },
        Kirigami.Action {
            text: i18n("Edit with Vespucci")
            icon.name: "document-edit"
            visible: Settings.osmContributorMode && EditorController.hasEditor(Editor.Vespucci)
            onTriggered: EditorController.editBoundingBox(root.map.view.mapSceneToGeo(root.map.view.viewport), Editor.Vespucci)
        },

        Kirigami.Action {
            id: lightStyleAction
            icon.name: "lighttable"
            text: "Light Style"
            onTriggered: map.styleSheet = "breeze-light"
            visible: Settings.developmentMode
        },
        Kirigami.Action {
            id: darkStyleAction
            icon.name: "lighttable"
            text: "Dark Style"
            onTriggered: map.styleSheet = "breeze-dark"
            visible: Settings.developmentMode
        },
        Kirigami.Action {
            id: diagnosticStyleAction
            icon.name: "tools-report-bug"
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
        parent: root.overlay
    }

    PlatformModel {
        id: platformModel
        mapData: map.mapData

        onPlatformIndexChanged: {
            if (platformModel.departurePlatformRow >= 0) {
                var idx = platformModel.index(platformModel.departurePlatformRow, 0);
                map.view.floorLevel = platformModel.data(idx, PlatformModel.LevelRole)
                map.view.centerOnGeoCoordinate(platformModel.data(idx, PlatformModel.CoordinateRole));
                map.view.setZoomLevel(19, Qt.point(map.width / 2.0, map.height / 2.0));
            } else if (root.departurePlatformName == "" && platformModel.arrivalPlatformRow >= 0) {
                var idx = platformModel.index(platformModel.arrivalPlatformRow, 0);
                map.view.floorLevel = platformModel.data(idx, PlatformModel.LevelRole)
                map.view.centerOnGeoCoordinate(platformModel.data(idx, PlatformModel.CoordinateRole));
                map.view.setZoomLevel(19, Qt.point(map.width / 2.0, map.height / 2.0));
            }
        }
    }

    IndoorMapPlatformSheet {
        id: platformSheet
        model: platformModel
        parent: root.overlay
    }

    GateModel {
        id: gateModel
        mapData: map.mapData

        onGateIndexChanged: {
            if (gateModel.departureGateRow >= 0) {
                var idx = gateModel.index(gateModel.departureGateRow, 0);
                map.view.floorLevel = gateModel.data(idx, GateModel.LevelRole)
                map.view.centerOnGeoCoordinate(gateModel.data(idx, GateModel.CoordinateRole));
                map.view.setZoomLevel(18, Qt.point(map.width / 2.0, map.height / 2.0));
            } else if (root.departureGateName == "" && platformModel.arrivalGateRow >= 0) {
                var idx = platformModel.index(platformModel.arrivalGateRow, 0);
                map.view.floorLevel = platformModel.data(idx, PlatformModel.LevelRole)
                map.view.centerOnGeoCoordinate(platformModel.data(idx, PlatformModel.CoordinateRole));
                map.view.setZoomLevel(19, Qt.point(map.width / 2.0, map.height / 2.0));
            }
        }
    }

    IndoorMapGateSheet {
        id: gateSheet
        model: gateModel
        parent: root.overlay
    }

    FloorLevelChangeModel {
        id: floorLevelChangeModel
        currentFloorLevel: map.view.floorLevel
        floorLevelModel: map.floorLevels
    }

    IndoorMapElevatorSheet {
        id: elevatorSheet
        model: floorLevelChangeModel
        parent: root.overlay
    }

    LocationQueryOverlayProxyModel {
        id: locationModel
        sourceModel: PT.LocationQueryModel {
            id: locationQuery
            manager: LiveDataManager.publicTransportManager
        }
        mapData: map.mapData
    }

    RealtimeEquipmentModel {
        id: equipmentModel
        mapData: map.mapData
        realtimeModel: locationModel.sourceModel
    }

    IndoorMapAmenitySheet {
        id: amenitySheet
        model: AmenityModel {
            mapData: map.mapData
        }
        map: root.map
    }

    function queryLiveLocationData()
    {
        if (rentalVehicleAction.checked || equipmentAction.checked) {
            locationQuery.request.latitude = map.mapData.center.y;
            locationQuery.request.longitude = map.mapData.center.x;
            locationQuery.request.maximumDistance = map.mapData.radius;
            locationQuery.request.types =
                (rentalVehicleAction.checked ? (PT.Location.RentedVehicleStation | PT.Location.RentedVehicle) : 0)
              | (equipmentAction.checked ? PT.Location.Equipment : 0);
        } else {
            locationQuery.clear();
        }
    }

    IndoorMap {
        id: map
        anchors.fill: parent
        overlaySources: [ gateModel, platformModel, locationModel, equipmentModel ]

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
                elevatorSheet.open();
                return;
            }

            infoModel.element = element;
            if (infoModel.name != "" || infoModel.debug) {
                elementDetailsSheet.open();
            }
        }
        onElementLongPressed: {
            // this provides info model access for elements with other interactions
            infoModel.element = element;
            if (infoModel.name != "" || infoModel.debug) {
                elementDetailsSheet.open();
            }
        }
    }

    onCoordinateChanged: map.mapLoader.loadForCoordinate(coordinate.y, coordinate.x);

    Connections {
        target: map.mapLoader
        function onDone() {
            gateModel.setArrivalGate(root.arrivalGateName);
            gateModel.setDepartureGate(root.departureGateName);
            map.region = root.region;
            map.timeZone = root.timeZone;
            map.view.beginTime = root.beginTime;
            map.view.endTime = root.endTime;
            queryLiveLocationData();
        }
    }

    Components.DoubleFloatingButton {
        anchors {
            right: parent.right
            rightMargin: Kirigami.Units.largeSpacing
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing // to not hide the copyright information
        }

        leftAction: Kirigami.Action {
            icon.name: "go-down-symbolic"
            text: i18nc("@action:intoolbar Go down one floor", "Floor down")
            enabled: map.floorLevels.hasFloorLevelBelow(map.view.floorLevel)
            onTriggered: map.view.floorLevel = map.floorLevels.floorLevelBelow(map.view.floorLevel)
            visible: map.floorLevels.hasFloorLevels
        }

        rightAction: Kirigami.Action {
            icon.name: "go-up-symbolic"
            text: i18nc("@action:intoolbar Go up one floor", "Floor up")
            enabled: map.floorLevels.hasFloorLevelAbove(map.view.floorLevel)
            onTriggered: map.view.floorLevel = map.floorLevels.floorLevelAbove(map.view.floorLevel)
            visible: map.floorLevels.hasFloorLevels
        }
    }
}
