// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls as QQC2
import org.kde.kirigamiaddons.formcard as FormCard
import QtQuick.Layouts
import org.kde.itinerary

QQC2.Pane {
    property alias day: _controller.date
    property QtObject controller: TimelineSectionDelegateController {
        id: _controller;
        tripGroupModel: TripGroupModel
    }

    width: ListView.view.width

    contentItem: RowLayout {
        Item{ Layout.fillWidth: true }
        RowLayout{
            Layout.margins: Kirigami.Units.smallSpacing
            Layout.maximumWidth: Kirigami.Units.gridUnit * 29
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
        Item{ Layout.fillWidth: true }
    }

    Accessible.name: titleLabel.text
}
