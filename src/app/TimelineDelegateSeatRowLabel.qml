// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: Copyright: Mathis Brüchert <mbb-mail@gmx.de>

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

QQC2.Label {
    id: root

    /** Prefer hiding this label rather than enabling eliding elsewhere when we run out of space */
    property bool lowPriority: false

    horizontalAlignment: Qt.AlignHCenter
    verticalAlignment: Qt.AlignVCenter
    Layout.fillWidth: true
    Layout.fillHeight: true

    // as long as we have enough space, prefer equally sized labels
    Layout.preferredWidth: parent.enableEliding ? implicitWidth : parent.width
    // hide low-priority content before resorting to eliding
    visible: !parent.hideLowPriorityContent || !root.lowPriority
    // when space gets limited, trade in equal sizeing first to avoid eliding
    Layout.minimumWidth: parent.enableEliding ? 0 : implicitWidth
    // elide as last resort, when there is not enough space for all labels
    elide: Text.ElideLeft
}
