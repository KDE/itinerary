/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.4 as Kirigami
import org.kde.itinerary 1.0
import "." as App

RowLayout {
    id: root
    property var statItem

    Kirigami.FormData.label: root.statItem.label

    Kirigami.Icon {
        source: root.statItem.trend == StatisticsItem.TrendUp ? "go-up-symbolic" : root.statItem.trend == StatisticsItem.TrendDown ? "go-down-symbolic" : "go-next-symbolic"
        color: root.statItem.trend == StatisticsItem.TrendUp ? Kirigami.Theme.negativeTextColor : root.statItem.trend == StatisticsItem.TrendDown ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.textColor
        width: height
        height: Kirigami.Units.gridUnit
        visible: root.statItem.trend != StatisticsItem.TrendUnknown
    }

    QQC2.Label {
        text: root.statItem.value
    }
}
