// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls as QQC2
import org.kde.kirigamiaddons.formcard as FormCard
import QtQuick.Layouts
import org.kde.itinerary

FormCard.FormHeader {
    id: root

    property alias day: _controller.date

    // use either tripGroupModel or specify regionCode yourself
    property alias tripGroupModel: _controller.tripGroupModel
    property alias regionCode: _controller.regionCode

    property QtObject controller: TimelineSectionDelegateController {
        id: _controller;
    }

    width: ListView.view.width
    title: controller.title
    topPadding: Kirigami.Units.smallSpacing

    Accessible.description: controller.subTitle

    trailing: QQC2.Label {
        topPadding: root.topPadding
        bottomPadding: root.bottomPadding
        leftPadding: root.leftPadding
        rightPadding: root.rightPadding

        Layout.fillWidth: true
        Layout.preferredWidth: root.width / 2

        text: controller.subTitle
        color: root.controller.isHoliday ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.disabledTextColor

        elide: Qt.ElideRight

        Accessible.ignored: true
    }
}
