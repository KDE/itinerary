// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: Copyright: Mathis Brüchert <mbb-mail@gmx.de>

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

QQC2.Label {
    horizontalAlignment: Qt.AlignHCenter
    verticalAlignment: Qt.AlignVCenter
    Layout.fillWidth: true
    Layout.fillHeight: true

    // as long as we have enough space, prefer equally sized labels
    Layout.preferredWidth: parent.width
    // when space gets limited, trade in equal sizeinf first to avoid eliding
    Layout.minimumWidth: parent.compactMode ? 0 : implicitWidth
    // elide as last resort, when there is not enough space for all labels
    elide: Text.ElideRight
}
