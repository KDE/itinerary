// SPDX-FleCopyrightText: 2019 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.itinerary 1.0
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import "." as App

MobileForm.FormTextDelegate {
    id: root
    property var statItem

    visible: statItem.hasData
    text: statItem.label
    description: statItem.value

    contentItem.children: [
        Kirigami.Icon {
            source: root.statItem.trend == StatisticsItem.TrendUp ? "go-up-symbolic" : root.statItem.trend == StatisticsItem.TrendDown ? "go-down-symbolic" : "go-next-symbolic"
            color: root.statItem.trend == StatisticsItem.TrendUp ? Kirigami.Theme.negativeTextColor : root.statItem.trend == StatisticsItem.TrendDown ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.textColor
            Layout.preferredWidth: Kirigami.Units.gridUnit
            Layout.preferredHeight: Kirigami.Units.gridUnit
            visible: root.statItem.trend != StatisticsItem.TrendUnknown
        }
    ]
}
