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
    }
}
