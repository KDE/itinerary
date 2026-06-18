// SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.pkpass as KPkPass

/*! Multi-row auxiliary field grid. */
ColumnLayout {
    id: root

    /*! The pass these fields are from. */
    property KPkPass.Pass pass: null
    /*! Fallback text color when not specified in the pass. */
    property color defaultTextColor: palette.text

    Layout.fillWidth: true

    Repeater {
        model: root.pass.auxiliaryFieldsRowCount
        GridLayout {
            id: auxFieldsLayout
            required property int index
            rows: 2
            columns: root.pass.auxiliaryFieldsInRow(index).length
            Layout.fillWidth: true

            Repeater {
                model: root.pass.auxiliaryFieldsInRow(auxFieldsLayout.index)
                delegate: FieldLabel {
                    required property KPkPass.field modelData
                    field: modelData
                    color: root.pass.hasLabelColor ? root.pass.labelColor : root.defaultTextColor
                }
            }
            Repeater {
                model: root.pass.auxiliaryFieldsInRow(auxFieldsLayout.index)
                delegate: FieldValue {
                    required property KPkPass.field modelData
                    field: modelData
                    color: root.pass.hasForegroundColor ? root.pass.foregroundColor : root.defaultTextColor
                }
            }
        }
    }
}
