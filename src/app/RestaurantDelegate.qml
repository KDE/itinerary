/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.itinerary 1.0
import "." as App

App.TimelineDelegate {
    id: root

    headerIconSource: "qrc:///images/foodestablishment.svg"
    headerItem: RowLayout {
        QQC2.Label {
            text: reservationFor.name
            color: Kirigami.Theme.textColor
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: Localizer.formatTime(reservation, "startTime")
            color: Kirigami.Theme.textColor
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
        }
    }

    contentItem: Column {
        id: topLayout
        spacing: Kirigami.Units.smallSpacing

        App.PlaceDelegate {
            place: reservationFor
            controller: root.controller
            width: topLayout.width
            showButtons: false
        }
    }

    onClicked: showDetailsPage(restaurantDetailsPage, root.batchId)
}
