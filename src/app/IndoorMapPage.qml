/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtNetwork as QtNetwork
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as PT
import org.kde.kosmindoormap
import org.kde.itinerary
import org.kde.kosmindoormap.kpublictransport
import org.kde.osm.editorcontroller

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

    actions: [
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
            enabled: !map.mapLoader.isLoading && (QtNetwork.NetworkInformation.reachability === QtNetwork.NetworkInformation.Reachability.Online || QtNetwork.NetworkInformation.reachability === QtNetwork.NetworkInformation.Reachability.Unknown)
            icon.color: QtNetwork.NetworkInformation.isMetered ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.textColor
            onTriggered: queryLiveLocationData();
        },
        Kirigami.Action {
            id: rentalVehicleAction
            icon.name: "car"
            text: i18n("Show Rental Vehicles")
            checkable: true
            enabled: !map.mapLoader.isLoading && (QtNetwork.NetworkInformation.reachability === QtNetwork.NetworkInformation.Reachability.Online || QtNetwork.NetworkInformation.reachability === QtNetwork.NetworkInformation.Reachability.Unknown)
            icon.color: QtNetwork.NetworkInformation.isMetered ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.textColor
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

    PlatformModel {
        id: platformModel
        mapData: map.mapData

        onPlatformIndexChanged: {
            let idx;
            if (platformModel.departurePlatformRow >= 0) {
                idx = platformModel.index(platformModel.departurePlatformRow, 0);
            } else if (root.departurePlatformName == "" && platformModel.arrivalPlatformRow >= 0) {
                idx = platformModel.index(platformModel.arrivalPlatformRow, 0);
            } else {
                return;
            }
            map.view.centerOn(platformModel.data(idx, PlatformModel.CoordinateRole), platformModel.data(idx, PlatformModel.LevelRole), 19);
        }
    }

    PlatformDialog {
        id: platformSheet
        model: platformModel
        parent: root.overlay
        onPlatformSelected: (platform) => { map.view.centerOn(platform.position, platform.level, 19); }
    }

    GateModel {
        id: gateModel
        mapData: map.mapData

        onGateIndexChanged: {
            let idx;
            if (gateModel.departureGateRow >= 0) {
                idx = gateModel.index(gateModel.departureGateRow, 0);
            } else if (root.departureGateName == "" && gateModel.arrivalGateRow >= 0) {
                idx = platformModel.index(gateModel.arrivalGateRow, 0);
            } else {
                return;
            }
            map.view.centerOn(gateModel.data(idx, GateModel.CoordinateRole), gateModel.data(idx, GateModel.LevelRole), 18);
        }
    }

    IndoorMapGateSheet {
        id: gateSheet
        model: gateModel
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

    AmenitySearchDialog {
        id: amenitySheet
        amenityModel: AmenityModel {
            mapData: map.mapData
        }
        onAmenitySelected: (amenity) => { map.view.centerOn(amenity.element.center, amenity.level, 21); }
    }

    function queryLiveLocationData()
    {
        if (rentalVehicleAction.checked || equipmentAction.checked) {
            locationQuery.request = {
                latitude: map.mapData.center.y,
                longitude: map.mapData.center.x,
                maximumDistance: map.mapData.radius,
                types: (rentalVehicleAction.checked ? (PT.Location.RentedVehicleStation | PT.Location.RentedVehicle) : 0)
                     | (equipmentAction.checked ? PT.Location.Equipment : 0)
            };
        } else {
            locationQuery.clear();
        }
    }

    IndoorMapView {
        id: map
        anchors.fill: parent

        equipmentModel: RealtimeEquipmentModel {
            mapData: map.mapData
            realtimeModel: locationModel.sourceModel
        }

        overlaySources: [ gateModel, platformModel, locationModel, map.equipmentModel ]

        elementInfoModel {
            allowOnlineContent: Settings.wikimediaOnlineContentEnabled
            debug: Settings.developmentMode
        }

        elementInfoDialog:  IndoorMapInfoSheet {
            model: map.elementInfoModel
            regionCode: root.map.mapData.regionCode
            timeZone: root.map.mapData.timeZone
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

    footer: null
}
