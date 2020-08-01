/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.7 as Kirigami
import org.kde.kpublictransport 1.0 as PublicTransport
import org.kde.kosmindoormap 1.0
import org.kde.itinerary 1.0

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
        }
    ]

    OSMElementInformationModel {
        id: infoModel
        //debug: true
    }

    IndoorMapInfoSheet {
        id: elementDetailsSheet
        model: infoModel
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

    IndoorMap {
        id: map
        anchors.fill: parent

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
        }
    }
}
