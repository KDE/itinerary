/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import org.kde.kirigami 2.17 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Item {
    id: root
    property alias day: _controller.date
    property QtObject controller: TimelineSectionDelegateController {
        id: _controller;
        timelineModel: TimelineModel
    }

    implicitHeight: headerItem.implicitHeight + Kirigami.Units.largeSpacing*2
    implicitWidth: parent.width
    Kirigami.BasicListItem {
        id: headerItem
        label: controller.title
        backgroundColor: Kirigami.Theme.backgroundColor // otherwise background is transparent
        icon: "view-calendar-day"
        iconColor: controller.isHoliday ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
        bold: controller.isToday
        subtitle: controller.subTitle
    }
}
