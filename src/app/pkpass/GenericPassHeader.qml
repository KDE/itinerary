/*
    SPDX-FileCopyrightText: 2018-2026 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
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
    columns: root.pass.headerFields.length + 2
    Layout.fillWidth: true

    Image {
        Layout.rowSpan: 2
        Layout.maximumHeight: 50
        Layout.maximumWidth: 160
        Layout.preferredWidth: paintedWidth
        fillMode: Image.PreserveAspectFit
        source: root.passId !== "" ? "image://org.kde.pkpass/" + root.passId + "/logo" : ""
        sourceSize.height: 1 // ??? seems necessary to trigger high dpi scaling...
    }

    QQC2.Label {
        Layout.rowSpan: 2
        Layout.fillWidth: true
        text: root.pass ? root.pass.logoText : ""
        color: root.pass.hasForegroundColor ? root.pass.foregroundColor : root.defaultTextColor
        font.bold: true
    }

    Repeater {
        model: root.pass.headerFields
        delegate: QQC2.Label {
            required property KPkPass.field modelData
            required property int index
            text: modelData.label
            color: root.pass.hasLabelColor ? root.pass.labelColor : root.defaultTextColor
            elide: Text.ElideRight
            // Layout.fillWidth: true
            Layout.alignment: index + 1 !== root.pass.headerFields.length ? modelData.textAlignment : Qt.AlignRight
            font: Kirigami.Theme.smallFont
        }
    }
    Repeater {
        model: root.pass.headerFields
        delegate: QQC2.Label {
            required property KPkPass.field modelData
            required property int index
            text: modelData.valueDisplayString
            color: root.pass.hasForegroundColor ? root.pass.foregroundColor : root.defaultTextColor
            elide: Text.ElideRight
            // Layout.fillWidth: true
            Layout.alignment: index + 1 !== root.pass.headerFields.length ? modelData.textAlignment : Qt.AlignRight
        }
    }
}
