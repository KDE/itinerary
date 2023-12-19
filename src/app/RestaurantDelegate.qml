/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.itinerary

TimelineDelegate {
    id: root

    headerIconSource: "qrc:///images/foodestablishment.svg"
    headerItem: RowLayout {
        QQC2.Label {
            id: headerLabel
            text: reservationFor.name
            color: root.headerTextColor
            elide: Text.ElideRight

            Layout.fillWidth: true
            Accessible.ignored: true
        }
        QQC2.Label {
            text: Localizer.formatTime(reservation, "startTime")
            color: root.headerTextColor
        }
    }

    contentItem: Column {
        id: topLayout
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            visible: text !== ""
            width: topLayout.width
            text: Localizer.formatAddressWithContext(reservationFor.address, null, Settings.homeCountryIsoCode)
        }
    }

    onClicked: showDetailsPage(restaurantDetailsPage, root.batchId)
    Accessible.name: headerLabel.text
}
