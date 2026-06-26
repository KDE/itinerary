/*
    SPDX-FileCopyrightText: 2018-2026 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.pkpass as KPkPass

/*! Header fields of regular event tickets. */
GridLayout {
    id: root

    /*! The pass these fields are from. */
    property KPkPass.Pass pass: null
    /*! Fallback text color when not specified in the pass. */
    property color defaultTextColor: palette.text

    /*! TODO can we get this from pass directly? */
    property string passId

    rows: 2
    columns: root.pass.headerFields.length + 2
    Layout.fillWidth: true

    Image {
        Layout.rowSpan: 2
        Layout.maximumHeight: 60
        Layout.maximumWidth: 150
        Layout.preferredWidth: paintedWidth
        fillMode: Image.PreserveAspectFit
        source: root.passId !== "" ? "image://org.kde.pkpass/" + root.passId + "/logo" : ""
        sourceSize.height: 1 // ??? seems necessary to trigger high dpi scaling...
        mipmap: true
    }

    QQC2.Label {
        Layout.rowSpan: 2
        Layout.fillWidth: true
        Layout.horizontalStretchFactor: 100
        text: root.pass ? root.pass.logoText : ""
        color: root.pass.hasForegroundColor ? root.pass.foregroundColor : root.defaultTextColor
    }

    Repeater {
        model: root.pass.headerFields
        delegate: FieldLabel {
            required property KPkPass.field modelData
            required property int index
            field: modelData
            color: root.pass.hasLabelColor ? root.pass.labelColor : root.defaultTextColor
            Layout.row: 0
            Layout.column: index + 2
            horizontalAlignment: index + 1 === root.pass.headerFields.length ? Qt.AlignRight : field.textAlignment
        }
    }
    Repeater {
        model: root.pass.headerFields
        delegate: FieldValue {
            required property KPkPass.field modelData
            required property int index
            field: modelData
            color: root.pass.hasForegroundColor ? root.pass.foregroundColor : root.defaultTextColor
            Layout.row: 1
            Layout.column: index + 2
            horizontalAlignment: index + 1 === root.pass.headerFields.length ? Qt.AlignRight : field.textAlignment
        }
    }
}
