/*
    SPDX-FileCopyrightText: 2018-2026 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.pkpass as KPkPass

/*! Header fields of poster event tickets. */
GridLayout {
    id: root

    /*! The pass these fields are from. */
    property KPkPass.Pass pass: null
    /*! Fallback text color when not specified in the pass. */
    property color defaultTextColor: palette.text

    /*! TODO can we get this from pass directly? */
    property string passId

    rows: 2
    columns: 3
    Layout.fillWidth: true

    Image {
        Layout.rowSpan: 2
        Layout.maximumHeight: 60
        Layout.maximumWidth: 150
        Layout.preferredWidth: paintedWidth
        fillMode: Image.PreserveAspectFit
        source: root.passId !== "" ? "image://org.kde.pkpass/" + root.passId + "/primaryLogo" : ""
        sourceSize.height: 1 // ??? seems necessary to trigger high dpi scaling...
        mipmap: true
    }

    QQC2.Label {
        Layout.rowSpan: 2
        Layout.fillWidth: true
        Layout.horizontalStretchFactor: 100
        // TODO concert and sport event variants
        text: root.pass ? root.pass.logoText : ""
        color: root.pass.hasForegroundColor ? root.pass.foregroundColor : root.defaultTextColor
    }

    QQC2.Label {
        Layout.row: 0
        Layout.column: 2
        Layout.fillWidth: true
        // TODO date ranges for multi-day events
        text: Qt.locale().toString(root.pass.semanticTags.eventStartDateInfo.date, "dd MMM")
        color: root.pass.hasForegroundColor ? root.pass.foregroundColor : root.defaultTextColor
        horizontalAlignment: Qt.AlignRight
        font.bold: true
    }
    QQC2.Label {
        Layout.row: 1
        Layout.column: 2
        Layout.fillWidth: true
        text: Qt.locale().toString(root.pass.semanticTags.eventStartDateInfo.date, "HH:mm")
        color: root.pass.hasForegroundColor ? root.pass.foregroundColor : root.defaultTextColor
        horizontalAlignment: Qt.AlignRight
    }

}
