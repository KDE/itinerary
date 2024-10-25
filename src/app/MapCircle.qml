/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtLocation as QtLocation
import QtPositioning as QtPositioning
import org.kde.kirigami as Kirigami
import org.kde.itinerary

/** Circle elements on the map. */
QtLocation.MapQuickItem {
    id: root

    property alias size: circle.height
    property alias borderWidth: circle.border.width
    property alias color: circle.border.color
    property alias textColor: circle.color
    property alias iconName: icon.source

    signal clicked()

    anchorPoint.x: sourceItem.width / 2
    anchorPoint.y: sourceItem.height / 2

    sourceItem: Rectangle {
        id: circle
        height: 15
        width: circle.height
        radius: height / 2
        border.width: 2

        Kirigami.Icon {
            id: icon
            isMask: true
            anchors.centerIn: circle
            height: root.size - root.borderWidth
            width: icon.height
            color: root.color
        }

        MouseArea {
            anchors.fill: parent
            scale: Math.max(1, 32 / root.height)
            onClicked: root.clicked()
        }
    }
}
