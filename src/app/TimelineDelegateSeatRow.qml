// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: Copyright: Mathis Brüchert <mbb-mail@gmx.de>

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Rectangle {
    Layout.topMargin: Kirigami.Units.smallSpacing
    Layout.bottomMargin: Kirigami.Units.largeSpacing
    border.width: 1
    border.color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.15)
    color: "transparent"
    Layout.fillWidth: true
    height: Kirigami.Units.gridUnit * 2
    radius: Kirigami.Units.smallSpacing

    default property alias __rowData: layout.data

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
