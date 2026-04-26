/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.Dialog {
    id: root

    title: i18nc("@title", "Select Calendar")

    property alias model: calendarSelectorListView.model

    signal calendarSelected(var calendar)

    width: Math.min(QQC2.ApplicationWindow.window.width, Kirigami.Units.gridUnit * 24)
    height: Math.min(QQC2.ApplicationWindow.window.height, Kirigami.Units.gridUnit * 32)

    contentItem: ListView {
        id: calendarSelectorListView
        clip: true

        delegate: QQC2.ItemDelegate {
            id: delegateRoot
            required property int index
            required property string name
            required property var calendar
            required property var modelData

            text: name
            width: ListView.view.width
            contentItem: RowLayout {
                Kirigami.IconTitleSubtitle {
                    title: delegateRoot.text
                    icon.source: delegateRoot.modelData.icon ?? "view-calendar-day"
                    Layout.fillWidth: true
                }
                Rectangle {
                    border {
                        color: Kirigami.Theme.textColor
                        width: 0.5
                    }
                    Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    // color property is only available with KF >= 6.26
                    Layout.preferredWidth: delegateRoot.modelData.hasOwnProperty("color") && delegateRoot.modelData.color ? Kirigami.Units.iconSizes.small : 0
                    color: delegateRoot.modelData.hasOwnProperty("color") ? delegateRoot.modelData.color : "transparent"
                    radius: Kirigami.Units.smallSpacing
                }
            }
            onClicked: {
                root.calendarSelected(calendar);
                root.close();
            }
        }
    }
}
