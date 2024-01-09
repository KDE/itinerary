// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls as QQC2
import org.kde.kirigamiaddons.formcard as FormCard
import QtQuick.Layouts
import org.kde.itinerary

// Use similar layout of FormCard but without background
Item {
    id: root

    property alias day: _controller.date
    property QtObject controller: TimelineSectionDelegateController {
        id: _controller;
        timelineModel: TimelineModel
    }

    readonly property bool cardWidthRestricted: root.width > root.maximumWidth
    property real maximumWidth: Kirigami.Units.gridUnit * 30
    property real padding: Kirigami.Units.largeSpacing
    property real verticalPadding: padding
    property real horizontalPadding: padding
    property real topPadding: verticalPadding
    property real bottomPadding: verticalPadding
    property real leftPadding: horizontalPadding
    property real rightPadding: horizontalPadding

    width: ListView.view.width
    implicitHeight: topPadding + bottomPadding + internalColumn.implicitHeight + rectangle.borderWidth * 2

    Rectangle {
        id: rectangle
        readonly property real borderWidth: 0

        color: Kirigami.Theme.backgroundColor

        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            right: parent.right

            leftMargin: root.cardWidthRestricted ? Math.round((root.width - root.maximumWidth) / 2) : -1
            rightMargin: root.cardWidthRestricted ? Math.round((root.width - root.maximumWidth) / 2) : -1
        }

        RowLayout{
            id: internalColumn

            spacing: Kirigami.Units.smallSpacing

            // add 1 to margins to account for the border (so content doesn't overlap it)
            anchors {
                fill: parent
                leftMargin: root.leftPadding + rectangle.borderWidth
                rightMargin: root.rightPadding + rectangle.borderWidth
                topMargin: root.topPadding + rectangle.borderWidth
                bottomMargin: root.bottomPadding + rectangle.borderWidth
            }

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
    }

    Accessible.name: titleLabel.text
}
