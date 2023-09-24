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
    readonly property var touristAttraction: ReservationManager.reservation(root.batchId).touristAttraction

    headerIconSource: "meeting-attending" // TODO better icon, e.g. something like 🎢?
    headerItem: RowLayout {
        QQC2.Label {
            text: touristAttraction.name
            color: root.headerTextColor
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: Localizer.formatTime(reservation, "arrivalTime")
            color: root.headerTextColor
        }
    }

    contentItem: Column {
        id: topLayout
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            visible: text !== ""
            width: topLayout.width
            text: Localizer.formatAddressWithContext(touristAttraction.address, null, Settings.homeCountryIsoCode)
        }
    }

//    onClicked: showDetailsPage(touristAttractionDetailsPage, root.batchId)
}
