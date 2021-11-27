/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.itinerary 1.0
import "." as App

RowLayout {
    id: root
    property var statItem
    property alias label: labelItem

    Kirigami.FormData.label: root.statItem.label
    Layout.fillWidth: true

    Kirigami.Icon {
        source: root.statItem.trend == StatisticsItem.TrendUp ? "go-up-symbolic" : root.statItem.trend == StatisticsItem.TrendDown ? "go-down-symbolic" : "go-next-symbolic"
        color: root.statItem.trend == StatisticsItem.TrendUp ? Kirigami.Theme.negativeTextColor : root.statItem.trend == StatisticsItem.TrendDown ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.textColor
        Layout.preferredWidth: Kirigami.Units.gridUnit
        Layout.preferredHeight: Kirigami.Units.gridUnit
        visible: root.statItem.trend != StatisticsItem.TrendUnknown
    }

    QQC2.Label {
        id: labelItem
        Layout.fillWidth: true
        text: root.statItem.value
        wrapMode: Text.Wrap
    }
}
