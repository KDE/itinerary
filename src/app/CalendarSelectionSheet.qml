/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: root
    title: i18n("Select Calendar")

    property alias model: calendarSelectorListView.model

    signal calendarSelected(var calendar)

    ListView {
        id: calendarSelectorListView
        implicitWidth: Kirigami.Units.gridUnit * 20
        delegate: Kirigami.BasicListItem {
            required property int index
            required property string name
            required property var calendar

            text: name
            onClicked: {
                root.calendarSelected(calendar);
                root.close();
            }
        }
    }
}
