/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components
import org.kde.itinerary

Kirigami.ScrollablePage {
    id: root

    property ImportController controller

    title: i18nc("@title", "Import")

    header: QQC2.Label {
        text: i18n("Selected items to import.")
    }

    ListView {
        id: stagingList
        model: root.controller

        delegate: QQC2.ItemDelegate {
            highlighted: model.selected
            width: ListView.view.width
            contentItem: RowLayout {
                Kirigami.IconTitleSubtitle {
                    icon.source: model.iconName
                    title: model.title
                    subtitle: model.subtitle

                    Layout.fillWidth: true
                }
                ColumnLayout {
                    Kirigami.Icon {
                        source: "meeting-attending"
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                        Layout.preferredHeight: Kirigami.Units.iconSizes.small
                        visible: model.batchSize > 0
                    }
                    Kirigami.Icon {
                        source: "mail-attachment-symbolic"
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                        Layout.preferredHeight: Kirigami.Units.iconSizes.small
                        visible: model.attachmentCount > 0
                    }

                }
            }
            onClicked: model.selected = !model.selected
        }

        FloatingButton {
            anchors {
                right: parent.right
                rightMargin: Kirigami.Units.largeSpacing + (root.contentItem.QQC2.ScrollBar && root.contentItem.QQC2.ScrollBar.vertical ? root.contentItem.QQC2.ScrollBar.vertical.width : 0)
                bottom: parent.bottom
                bottomMargin: Kirigami.Units.largeSpacing
            }
            action: Kirigami.Action {
                icon.name: "document-open"
                text: i18n("Import selection")
                enabled: root.controller.hasSelection
                onTriggered: {
                    ApplicationController.commitImport(ImportController);
                    if (stagingList.count === 0) {
                        applicationWindow().pageStack.layers.pop();
                    }
                }
            }
        }
    }

    Component.onDestruction: root.controller.clear()
}
