/*
    SPDX-FileCopyrightText: © 2025 Volker Krause <vkrause@kdeorg>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.coreaddons as CoreAddons

Kirigami.Dialog {
    title: i18n("What's new")

    width: Math.min(QQC2.ApplicationWindow.window.width, Kirigami.Units.gridUnit * 24)
    height: Math.min(QQC2.ApplicationWindow.window.height, Kirigami.Units.gridUnit * 32)

    Component {
        id: releaseDelegate
        ColumnLayout {
            id: delegateRoot
            required property CoreAddons.aboutRelease modelData
            width: ListView.view.width
            RowLayout {
                Kirigami.Heading {
                    text: delegateRoot.modelData.version
                    level: 3
                    Layout.alignment: Qt.AlignBottom | Qt.AlignLeft
                }
                QQC2.Label {
                    opacity: 0.6
                    font: Kirigami.Theme.smallFont
                    text: CoreAddons.Format.formatRelativeDate(delegateRoot.modelData.date, Locale.ShortFormat)
                    Layout.alignment: Qt.AlignBottom | Qt.AlignLeft
                }
            }
            QQC2.Label {
                text: delegateRoot.modelData.description
                wrapMode: Text.WordWrap

                Layout.fillWidth: true
            }
            Kirigami.UrlButton {
                visible: url !== ""
                url: delegateRoot.modelData.url
            }
        }
    }

    contentItem: Kirigami.Padding {
        horizontalPadding: Kirigami.Units.largeSpacing
        contentItem: ListView {
            clip: true
            delegate: releaseDelegate
            model: CoreAddons.AboutData.releases
            spacing: Kirigami.Units.largeSpacing
        }
    }
}
