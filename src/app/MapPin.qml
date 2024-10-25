/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtLocation as QtLocation
import QtPositioning as QtPositioning
import org.kde.kirigami as Kirigami
import org.kde.itinerary

/** Icon in pin element for the map. */
QtLocation.MapQuickItem {
    id: root

    property alias color: mainIcon.color
    property alias textColor: bgRect.color
    property alias iconName: subIcon.source
    signal clicked()

    anchorPoint.x: sourceItem.width / 2
    anchorPoint.y: sourceItem.height

    sourceItem: Kirigami.Icon {
        id: mainIcon

        height: Kirigami.Units.iconSizes.huge
        width: height
        source: "map-symbolic"
        isMask: true
        color: modelData.color

        Rectangle {
            id: bgRect
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -parent.height / 8
            width: height
            height: parent.height / 3 + 1
            radius: height / 2
        }
        Kirigami.Icon {
            id: subIcon
            anchors.centerIn: mainIcon
            anchors.verticalCenterOffset: -mainIcon.height / 8
            width: height
            height: parent.height / 3 + 1
            source: modelData.iconName
            isMask: true
            color: root.color
        }
        MouseArea {
            anchors.fill: parent
            onClicked: root.clicked()
        }
    }
}
