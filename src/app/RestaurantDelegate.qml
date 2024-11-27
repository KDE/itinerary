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

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing
        RowLayout {
            spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

            Layout.fillWidth: true

            TransportIcon {
                size: Kirigami.Units.iconSizes.smallMedium
                source: ReservationHelper.defaultIconName(root.reservation)
            }

            Kirigami.Heading {
                id: headerLabel
                level: 3
                text: reservationFor.name
                elide: Text.ElideRight

                Layout.fillWidth: true
                Accessible.ignored: true
            }

            Kirigami.Heading {
                text: Localizer.formatTime(reservation, "startTime")
                level: 2
            }
        }

        QQC2.Label {
            visible: text !== ""
            text: Localizer.formatAddressWithContext(reservationFor.address, null, Settings.homeCountryIsoCode)

            Layout.fillWidth: true
        }
    }

    Accessible.name: headerLabel.text
}
