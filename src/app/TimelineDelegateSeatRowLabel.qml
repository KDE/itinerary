// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: Copyright: Mathis Brüchert <mbb-mail@gmx.de>

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

Item {
    Layout.fillWidth: true
    Layout.fillHeight: true

    property alias text: label.text

    QQC2.Label {
        id: label
        anchors.centerIn: parent
    }
}
