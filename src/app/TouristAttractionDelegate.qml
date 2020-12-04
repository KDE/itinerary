/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.4 as Kirigami
import org.kde.itinerary 1.0
import "." as App

App.TimelineDelegate {
    id: root
    readonly property var touristAttraction: ReservationManager.reservation(root.batchId).touristAttraction

    headerIconSource: "meeting-attending" // TODO better icon, e.g. something like 🎢?
    headerItem: RowLayout {
        QQC2.Label {
            text: touristAttraction.name
            color: Kirigami.Theme.textColor
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: Localizer.formatTime(reservation, "arrivalTime")
            color: Kirigami.Theme.textColor
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
        }
    }

    contentItem: Column {
        id: topLayout
        spacing: Kirigami.Units.smallSpacing

        App.PlaceDelegate {
            place: touristAttraction
            controller: root.controller
            width: topLayout.width
        }
    }

    Component {
        id: detailsComponent
        App.TouristAttractionPage {
            batchId: root.batchId
        }
    }

    onClicked: applicationWindow().pageStack.push(detailsComponent);
}
