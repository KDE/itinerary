/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.12 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.DetailsPage {
    id: root
    readonly property var touristAttraction: ReservationManager.reservation(root.batchId).touristAttraction
    title: i18n("Tourist Attraction")

    Kirigami.FormLayout {
        width: root.width

        QQC2.Label {
            Kirigami.FormData.isSection: true
            text: touristAttraction.name
            horizontalAlignment: Qt.AlignHCenter
            font.bold: true
        }

        App.PlaceDelegate {
            Kirigami.FormData.label: i18n("Location:")
            place: touristAttraction
            controller: root.controller
        }

        QQC2.Label {
            Kirigami.FormData.label: i18n("Arrival Time:")
            text: Localizer.formatDateTime(reservation, "arrivalTime")
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Departure Time:")
            text: Localizer.formatDateTime(reservation, "departureTime")
        }
    }
}

