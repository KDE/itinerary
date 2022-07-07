/*
 *   SPDX-FileCopyrightText: 2012 Viranch Mehta <viranch.mehta@gmail.com>
 *   SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
 *   SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.SvgItem {
    id: secondHand

    property alias rotation: rotation.angle
    property double svgScale

    width: Math.round(naturalSize.width * svgScale) + Math.round(naturalSize.width * svgScale) % 2
    height: Math.round(naturalSize.height * svgScale) + width % 2
    anchors {
        top: clock.verticalCenter
        topMargin: -width/2
        horizontalCenter: clock.horizontalCenter
    }

    svg: clockSvg
    smooth: !anim.running
    transform: Rotation {
        id: rotation
        angle: 0
        origin {
            x: secondHand.width/2
            y: secondHand.width/2
        }
        Behavior on angle {
            RotationAnimation {
                id: anim
                duration: 200
                direction: RotationAnimation.Clockwise
                easing.type: Easing.OutElastic
                easing.overshoot: 0.5
            }
        }
    }
}
