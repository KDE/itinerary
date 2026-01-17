/*
    SPDX-FileCopyrightText: © 2025 Volker Krause <vkrause@kdeorg>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.coreaddons as CoreAddons

Kirigami.Dialog {
    id: root
    title: i18n("What's new")

    property alias hasReleases: controller.hasReleases
    property alias hasNewReleases: controller.hasNewReleases

    width: Math.min(QQC2.ApplicationWindow.window.width, Kirigami.Units.gridUnit * 24)
    height: Math.min(QQC2.ApplicationWindow.window.height, Kirigami.Units.gridUnit * 32)

    readonly property WhatsNewController __controller: WhatsNewController {
        id: controller
        allReleases: CoreAddons.AboutData.releases
    }
    onVisibleChanged: if (visible) controller.updateLastSeenVersion()

    property bool showOnlyNew: false
    Component.onCompleted: root.showOnlyNew = controller.hasNewReleases // don't bind

    Component {
        id: releaseDelegate
        ColumnLayout {
            id: delegateRoot
            required property CoreAddons.aboutRelease modelData
            width: ListView.view.width - Kirigami.Units.largeSpacing
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

    contentItem: QQC2.ScrollView {
            QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff
            leftPadding: Kirigami.Units.largeSpacing
            ListView {
                id: listView
                clip: true
                delegate: releaseDelegate
                model: root.showOnlyNew ? controller.newReleases : controller.allReleases
                spacing: Kirigami.Units.largeSpacing
                footer: RowLayout {
                    width: listView.width
                    QQC2.Button {
                        text: i18n("Show older changes…")
                        visible: root.showOnlyNew
                        Layout.alignment: Qt.AlignHCenter
                        onClicked: root.showOnlyNew = false
                    }
                }
            }
    }
}
