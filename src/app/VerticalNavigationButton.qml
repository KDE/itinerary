// SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

ColumnLayout {
    id: root

    required property string iconName
    required property string text

    signal clicked

    spacing: 0

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: root.visible ? Kirigami.Units.largeSpacing : 0
        Layout.bottomMargin: root.visible ? Kirigami.Units.largeSpacing : 0

        FormCard.AbstractFormDelegate {
            id: button

            onClicked: root.clicked()
            text: root.text

            contentItem: RowLayout {
                Kirigami.Icon {
                    source: root.iconName
                    implicitWidth: Kirigami.Units.iconSizes.small
                    implicitHeight: Kirigami.Units.iconSizes.small

                    Layout.rightMargin: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                }

                QQC2.Label {
                    text: root.text
                    Layout.fillWidth: true
                }
            }

            Layout.fillWidth: true
            Accessible.ignored: true
        }

        Accessible.onPressAction: root.clicked()
    }
}

