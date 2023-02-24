/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

/** Line segment drawn on the left in the journey section view.
 *  Can be used in a fully drawn form, or partially drawn as a progress
 *  overlay over the full variant.
 */
Item {
    id: lineSegment

    property bool isDeparture: false
    property bool isArrival: false
    property color lineColor: Kirigami.Theme.textColor
    property int lineWidth: Kirigami.Units.smallSpacing

    property bool hasStop: true

    implicitWidth: lineSegment.lineWidth * 5

    readonly property real leadingLineLength: leadingLine.height
    readonly property real trailingLineLength: trailingLine.height

    readonly property bool isIntermediate: !isDeparture && !isArrival

    Rectangle {
        id: leadingLine
        x: 2 * lineSegment.lineWidth
        width: lineSegment.lineWidth
        color: lineSegment.lineColor
        height: parent.height / 2
        visible: !isDeparture
    }
    Rectangle {
        id: trailingLine
        x: 2 * lineSegment.lineWidth
        y: height
        width: lineSegment.lineWidth
        color: lineSegment.lineColor
        height: parent.height / 2
        visible: !isArrival
    }

    Rectangle {
        id: stopDot
        radius: width / 2
        width: lineSegment.lineWidth * (isIntermediate ? 4 : 5)
        height: width
        border {
            width: lineSegment.lineWidth
            color: lineSegment.lineColor
        }
        color: Kirigami.Theme.backgroundColor
        anchors.centerIn: parent
        visible: !isIntermediate || stop.disruptionEffect != Disruption.NoService
    }
}
