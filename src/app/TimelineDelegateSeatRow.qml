// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: Copyright: Mathis Brüchert <mbb-mail@gmx.de>

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

RowLayout {
    id: root

    required property var route
    required property bool hasSeat
    property real progress

    default property alias __rowData: layout.data

    visible: root.hasSeat
    spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

    Layout.fillWidth: true

    JourneySectionStopDelegateLineSegment {
        lineColor: root.route.line.hasColor ? root.route.line.color : Kirigami.Theme.textColor
        isDeparture: false
        hasStop: false

        leadingProgress: root.progress > 0 ? 1.0 : 0;
        trailingProgress: root.progress > 0 ? 1.0 : 0;

        Layout.fillHeight: true
    }

    Rectangle {
        height: Kirigami.Units.gridUnit * 2
        radius: Kirigami.Units.smallSpacing
        color: "transparent"
        border {
            width: 1
            color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.15)
        }

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.smallSpacing
        Layout.bottomMargin: Kirigami.Units.largeSpacing

        RowLayout{
            id: layout

            anchors.fill: parent

            // not enough space for all labels even with giving up equal sizing, so hide low-priority content
            readonly property bool hideLowPriorityContent: {
                let width = 0;
                for (const child of layout.children)
                    width += child.implicitWidth + 2 * Kirigami.Units.smallSpacing;
                return width > layout.width;
            }

            // not enough space with optional content gone either, so enable eliding as a last resort
            readonly property bool enableEliding: {
                let width = 0;
                for (const child of layout.children) {
                    if (!child.hasOwnProperty("lowPriority") || !child.lowPriority)
                        width += child.implicitWidth + 2 * Kirigami.Units.smallSpacing;
                }
                return width > layout.width;
            }
        }
    }
}
