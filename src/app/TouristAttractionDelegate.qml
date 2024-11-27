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

    readonly property var touristAttraction: ReservationManager.reservation(root.batchId).touristAttraction

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

            Layout.fillWidth: true

            TransportIcon {
                size: Kirigami.Units.iconSizes.smallMedium
                source: "meeting-attending"
            }

            Kirigami.Heading {
                text: touristAttraction.name
                level: 3
                Layout.fillWidth: true
            }

            Kirigami.Heading {
                level: 2
                text: Localizer.formatTime(reservation, "arrivalTime")
                color: root.headerTextColor
            }
        }

        QQC2.Label {
            Layout.fillWidth: true
            visible: text !== ""
            text: Localizer.formatAddressWithContext(touristAttraction.address, null, Settings.homeCountryIsoCode)
        }
    }
}
