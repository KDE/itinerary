/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtLocation as QtLocation
import QtPositioning
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.kirigamiaddons.components
import org.kde.itinerary

Kirigami.Page {
    id: root

    /** Initially selected coordinate, if any. */
    property variant coordinate

    /** Initially shown country, if coorinate is invalid. */
    property string country

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    // prevent swipe to the right changing pages, we want to pan the map instead
    Kirigami.ColumnView.preventStealing: true

    MapView {
        id: map
        anchors.fill: parent

        QtLocation.MapQuickItem {
            coordinate: map.center
            anchorPoint { x: icon.width / 2; y: icon.height / 2 }
            sourceItem: Kirigami.Icon {
                id: icon
                source: "crosshairs"
                width: height
                height: Kirigami.Units.iconSizes.large
                color: Kirigami.Theme.negativeTextColor
            }
        }

        // the zoom level computation only works once we have a proper view size
        onHeightChanged: initialMapPosition()
        onWidthChanged: initialMapPosition()

        onZoomLevelChanged: console.log("zoom", map.zoomLevel)
    }

    function initialMapPosition() {
        if (root.coordinate !== undefined && root.coordinate.isValid) {
            map.center = root.coordinate
            map.zoomLevel = 15
        } else if (root.country.length === 2) {
            const bbox = KPublicTransport.MapUtils.boundingBoxForCountry(root.country);
            map.center = KPublicTransport.MapUtils.center(bbox);
            map.zoomLevel = KPublicTransport.MapUtils.zoomLevel(bbox, map.width, map.height);
        }
    }

    Component.onCompleted: initialMapPosition()

    FloatingButton {
        anchors {
            right: parent.right
            rightMargin: Kirigami.Units.largeSpacing + (root.contentItem.QQC2.ScrollBar && root.contentItem.QQC2.ScrollBar.vertical ? root.contentItem.QQC2.ScrollBar.vertical.width : 0)
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.largeSpacing
        }
        action: Kirigami.Action {
            icon.name: "crosshairs"
            text: i18n("Pick Location")
            onTriggered: {
                coordinate = map.center;
                applicationWindow().pageStack.goBack();
            }
        }
    }

    footer: null
}
