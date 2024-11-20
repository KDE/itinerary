/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport
import org.kde.itinerary

/** Line segment drawn on the left in the journey section view.
 *  Can be used in a fully drawn form, or partially drawn as a progress
 *  overlay over the full variant.
 */
Item {
    id: lineSegment
    clip: true
    property bool isDeparture: false
    property bool isArrival: false
    property color lineColor: Kirigami.Theme.textColor
    property int lineWidth: Kirigami.Units.smallSpacing * 4

    property real leadingProgress: NaN
    property real trailingProgress: NaN
    property bool hasStop: true
    property bool showStop: lineSegment.hasStop

    implicitWidth: Kirigami.Units.iconSizes.smallMedium

    readonly property real leadingLineLength: line.height / 2
    readonly property real trailingLineLength: line.height / 2

    readonly property bool isIntermediate: !isDeparture && !isArrival

    Kirigami.ShadowedRectangle {
        id: line
        x: Math.round((lineSegment.implicitWidth - lineSegment.lineWidth) / 2)
        y: isDeparture? parent.height-height:0
        width: lineSegment.lineWidth
        color: lineSegment.lineColor

        corners {
            topRightRadius: isDeparture ? Math.round(width / 2) : 0
            topLeftRadius: isDeparture ? Math.round(width / 2) : 0

            bottomRightRadius: isArrival ? Math.round(width / 2) : 0
            bottomLeftRadius: isArrival ? Math.round(width / 2) : 0
        }
        height:
            if (isArrival) {
                Math.round(parent.height / 2) + lineSegment.lineWidth / 2
            } else if (isDeparture) {
                Math.round(parent.height / 2) + lineSegment.lineWidth / 2
            } else {
                parent.height
            }
    }
    Kirigami.ShadowedRectangle {
        id: progress
        x: line.x + (line.width - width) /2
        y: line.y + (isDeparture ? (line.width - width) / 2 + 0.3 * line.width: 0)
        width: lineSegment.lineWidth * 0.6
        color: Qt.platform.os !== "android" ? Kirigami.Theme.hoverColor : Kirigami.Theme.highlightColor
        opacity: 0.9
        height: ((isDeparture || isArrival) ? line.height - (line.width - width) / 2 - line.width * 0.3 : line.height) *
                (
                    isDeparture ? trailingProgress :
                    isArrival ? leadingProgress :
                    (leadingProgress + trailingProgress) * 0.5
                )
    }


    Rectangle {
        id: stopDot
        x: line.x + (line.width - width) / 2
        y: parent.height / 2 - width / 2
        radius: width / 2
        width: lineSegment.lineWidth * (isIntermediate ? 0.3 : 0.6)
        height: width
        color: Kirigami.Theme.backgroundColor
        opacity: isIntermediate ? 0.5:1
        visible:lineSegment.hasStop
    }
}
