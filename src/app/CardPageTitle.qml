// SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

ColumnLayout {
    id: root

    spacing: 0

    required property string emojiIcon
    required property string text

    Layout.fillWidth: true
    Layout.maximumWidth: Kirigami.Units.gridUnit * 30
    Layout.alignment: Qt.AlignHCenter

    QQC2.Label {
        text: root.emojiIcon
        horizontalAlignment: Text.AlignHCenter

        font {
            family: "emoji"
            pointSize: 40
        }

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter
        Accessible.ignored: true
    }

    Kirigami.Heading {
        text: root.text
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter

        leftPadding: Kirigami.Units.smallSpacing
        rightPadding: Kirigami.Units.smallSpacing
        bottomPadding: Kirigami.Units.smallSpacing

        Layout.fillWidth: true
    }
}
