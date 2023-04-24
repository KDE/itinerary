/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami

Kirigami.ShadowedRectangle {
    id: root

    property var card

    // see Kirigami.DefaultCardBackground
    property color defaultColor: Kirigami.Theme.backgroundColor
    property color pressedColor: Kirigami.ColorUtils.tintWithAlpha(defaultColor, Kirigami.Theme.highlightColor, 0.3)
    property color hoverColor: Kirigami.ColorUtils.tintWithAlpha(defaultColor, Kirigami.Theme.highlightColor, 0.1)
    property color borderColor: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.2)

    border {
        width: 1
        color: root.borderColor
    }
    corners {
        topLeftRadius: Kirigami.Units.smallSpacing
        topRightRadius: Kirigami.Units.smallSpacing
    }
    color: {
        if (root.card.checked || (root.card.showClickFeedback && (root.card.down || root.card.highlighted)))
            return root.pressedColor
        else if (root.card.hoverEnabled && root.card.hovered)
            return root.hoverColor
        return root.defaultColor
    }

    anchors.leftMargin: -root.card.leftPadding
    anchors.topMargin: -root.card.topPadding
    anchors.rightMargin: -root.card.rightPadding
}
