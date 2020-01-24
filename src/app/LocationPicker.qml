/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import QtLocation 5.11 as QtLocation
import QtPositioning 5.11
import org.kde.kirigami 2.4 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.Page {
    id: root
    property variant coordinate: QtPositioning.coordinate(0, 0)

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

   actions.main: Kirigami.Action {
        icon.name: "crosshairs"
        text: "Pick Location"
        onTriggered: {
            coordinate = map.center;
            applicationWindow().pageStack.pop();
        }
    }

    QtLocation.Plugin {
        id: mapPlugin
        required.mapping: QtLocation.Plugin.AnyMappingFeatures
        preferred: ["osm"]
    }

    QtLocation.Map {
        id: map
        anchors.fill: parent
        center: root.coordinate
        plugin: mapPlugin

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
