// SPDX-FleCopyrightText: 2019 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.itinerary
import org.kde.kirigamiaddons.formcard as FormCard
import "." as App

FormCard.FormTextDelegate {
    id: root
    property var statItem

    visible: statItem.hasData
    text: statItem.label
    description: statItem.value

    trailing: Kirigami.Icon {
        source: root.statItem.trend == StatisticsItem.TrendUp ? "go-up-symbolic" : root.statItem.trend == StatisticsItem.TrendDown ? "go-down-symbolic" : "go-next-symbolic"
        color: root.statItem.trend == StatisticsItem.TrendUp ? Kirigami.Theme.negativeTextColor : root.statItem.trend == StatisticsItem.TrendDown ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.textColor
        Layout.preferredWidth: Kirigami.Units.gridUnit
        Layout.preferredHeight: Kirigami.Units.gridUnit
        visible: root.statItem.trend != StatisticsItem.TrendUnknown
        implicitHeight: Kirigami.Units.gridUnit
        implicitWidth: Kirigami.Units.gridUnit
    }
}
