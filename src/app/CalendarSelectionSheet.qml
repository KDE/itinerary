/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.Dialog {
    id: root

    title: i18nc("@title", "Select Calendar")

    property alias model: calendarSelectorListView.model

    signal calendarSelected(var calendar)

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
    height: Math.min(applicationWindow().height, Kirigami.Units.gridUnit * 32)

    contentItem: ListView {
        id: calendarSelectorListView

        delegate: QQC2.ItemDelegate {
            required property int index
            required property string name
            required property var calendar

            text: name
            width: ListView.view.width
            contentItem: Kirigami.TitleSubtitle {
                title: parent.text
            }
            onClicked: {
                root.calendarSelected(calendar);
                root.close();
            }
        }
    }
}
