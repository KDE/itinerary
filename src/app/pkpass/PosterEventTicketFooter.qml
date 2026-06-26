/*
    SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.pkpass as KPkPass

/*! Header fields of poster event tickets. */
RowLayout {
    id: root

    /*! The pass these fields are from. */
    property KPkPass.Pass pass: null
    /*! Fallback text color when not specified in the pass. */
    property color defaultTextColor: palette.text

    /*! TODO can we get this from pass directly? */
    property string passId

    Layout.fillWidth: true

    QQC2.Label {
        text: root.pass.lookupMessage(root.pass.semanticTags.venueName)
        font: Kirigami.Theme.smallFont
        color: root.pass.hasLabelColor ? root.pass.labelColor : root.defaultTextColor
        Layout.fillWidth: true
    }

    Image {
        Layout.maximumHeight: 12
        Layout.minimumWidth: 12
        Layout.maximumWidth: 135
        Layout.preferredWidth: paintedWidth
        fillMode: Image.PreserveAspectFit
        source: root.passId !== "" ? "image://org.kde.pkpass/" + root.passId + "/secondaryLogo" : ""
        sourceSize.height: 1 // ??? seems necessary to trigger high dpi scaling...
        mipmap: true
    }
}
