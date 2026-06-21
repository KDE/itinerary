// SPDX-FileCopyrightText: 2018-2026 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.pkpass as KPkPass
import org.kde.itinerary

/*! Back fields. */
ColumnLayout {
    id: root

    /*! The pass these fields are from. */
    property KPkPass.Pass pass: null
    /*! Fallback text color when not specified in the pass. */
    property color defaultTextColor: palette.text

    Layout.fillWidth: true

    Repeater {
        model: root.pass.backFields
        ColumnLayout {
            id: delegateRoot
            required property KPkPass.field modelData
            FieldLabel {
                field: delegateRoot.modelData
                color: root.pass.hasLabelColor ? root.pass.labelColor : root.defaultTextColor
                wrapMode: Text.WordWrap
            }
            FieldValue {
                field: delegateRoot.modelData
                color: root.pass.hasForegroundColor ? root.pass.foregroundColor : root.defaultTextColor
                linkColor: root.pass.hasForegroundColor ? color : Kirigami.Theme.linkColor
                text: Util.textToHtml(delegateRoot.modelData.valueDisplayString)
                textFormat: Util.isRichText(delegateRoot.modelData.valueDisplayString) ? Text.StyledText : Text.AutoText
                wrapMode: Text.WordWrap
                onLinkActivated: (link) =>  { Qt.openUrlExternally(link); }
            }
        }
    }
}
