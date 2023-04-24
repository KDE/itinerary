// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import org.kde.kirigami 2.17 as Kirigami
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.itinerary 1.0
import "." as App

QQC2.Pane {
    property alias day: _controller.date
    property QtObject controller: TimelineSectionDelegateController {
        id: _controller;
        timelineModel: TimelineModel
    }

    leftPadding: 0
    rightPadding: 0
    topPadding: Kirigami.Units.smallSpacing
    bottomPadding: Kirigami.Units.smallSpacing

    width: ListView.view.width - ListView.view.parent.QQC2.ScrollBar.vertical.width

    contentItem: RowLayout {
        Kirigami.Icon {
            source: "view-calendar-day"
            color: controller.isHoliday ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
            isMask: controller.isHoliday
            implicitHeight: Kirigami.Units.iconSizes.smallMedium
            implicitWidth: Kirigami.Units.iconSizes.smallMedium
            Layout.alignment: Qt.AlignTop
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0
            Kirigami.Heading {
                id: titleLabel
                text: controller.title
                type: Kirigami.Heading.Type.Secondary
                font.weight: controller.isToday === Kirigami.Heading.Type.Primary ? Font.DemiBold : Font.Normal
                Layout.fillWidth: true
                level: 4
                Accessible.ignored: true
            }
            QQC2.Label {
                Layout.fillWidth: true
                text: controller.subTitle
                visible: text
                Accessible.ignored: !visible
                Layout.bottomMargin: Kirigami.Units.smallSpacing
            }
        }
    }

    Accessible.name: titleLabel.text
}
