/*
    SPDX-FileCopyrightText: 2019-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtPositioning
import QtLocation as QtLocation

import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami


/** QtLocation map view with standard interaction settings. */
QtLocation.Map {
    id: map
    plugin: applicationWindow().mapPlugin()

    copyrightsVisible: !applicationWindow().hasMapLibre
    onCopyrightLinkActivated: Qt.openUrlExternally(link)

    Rectangle {
        color: "#B0ffffff"
        anchors.fill: attributionLabel
        radius: Kirigami.Units.cornerRadius

        z: 100
    }

    Controls.Label {
        id: attributionLabel
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 2

        visible: applicationWindow().hasMapLibre

        textFormat: Text.RichText
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignLeft

        width: Kirigami.Units.gridUnit * 11

        text: i18n('<a href="https://openfreemap.org" target="_blank">OpenFreeMap</a> <a href="https://www.openmaptiles.org/" target="_blank">© OpenMapTiles</a><br>Data from <a href="https://www.openstreetmap.org/copyright" target="_blank">OpenStreetMap</a>')
        onLinkActivated: (link) => { Qt.openUrlExternally(link); }

        z: 101
    }

    property geoCoordinate startCentroid
    PinchHandler {
        id: pinch
        target: null
        onActiveChanged: if (active) {
            map.startCentroid = map.toCoordinate(pinch.centroid.position, false)
        }
        onScaleChanged: (delta) => {
            map.zoomLevel += Math.log2(delta)
            map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position)
        }
        xAxis.enabled: false
        yAxis.enabled: false
        minimumRotation: 0.0
        maximumRotation: 0.0
    }
    WheelHandler {
        id: wheel
        rotationScale: 1/120
        orientation: Qt.Vertical
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        onWheel: (event) => {
            map.startCentroid = map.toCoordinate(wheel.point.position, false)
            map.zoomLevel += event.angleDelta.y * rotationScale
            map.alignCoordinateToPoint(map.startCentroid, wheel.point.position)
        }
    }
    DragHandler {
        id: drag
        target: null
        onTranslationChanged: (delta) => map.pan(-delta.x, -delta.y)
    }
    Shortcut {
        enabled: map.zoomLevel < map.maximumZoomLevel
        sequence: StandardKey.ZoomIn
        onActivated: map.zoomLevel = Math.round(map.zoomLevel + 1)
    }
    Shortcut {
        enabled: map.zoomLevel > map.minimumZoomLevel
        sequence: StandardKey.ZoomOut
        onActivated: map.zoomLevel = Math.round(map.zoomLevel - 1)
    }
}
