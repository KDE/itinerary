// SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

ColumnLayout {
    id: root

    required property string iconName
    required property string text

    signal clicked

    spacing: 0

    MobileForm.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: root.visible ? Kirigami.Units.largeSpacing : 0
        Layout.bottomMargin: root.visible ? Kirigami.Units.largeSpacing : 0

        contentItem: ColumnLayout {
            spacing: 0

            MobileForm.AbstractFormDelegate {
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
            }
        }
    }
}

