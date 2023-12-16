/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtLocation as QtLocation
import QtPositioning
import org.kde.kirigami as Kirigami
import org.kde.itinerary
import "." as App

Kirigami.Page {
    id: root
    property variant coordinate: QtPositioning.coordinate(0, 0)

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    // prevent swipe to the right changing pages, we want to pan the map instead
    Kirigami.ColumnView.preventStealing: true

   actions.main: Kirigami.Action {
        icon.name: "crosshairs"
        text: i18n("Pick Location")
        onTriggered: {
            coordinate = map.center;
            applicationWindow().pageStack.goBack();
        }
    }

    MapView {
        id: map
        anchors.fill: parent
        center: root.coordinate
        zoomLevel: root.coordinate.isValid ? 15 : 8

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
    }
}
