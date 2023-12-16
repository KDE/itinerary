// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitinerary
import org.kde.itinerary
import "." as App

App.DetailsPage {
    id: root

    readonly property var touristAttraction: ReservationManager.reservation(root.batchId).touristAttraction

    title: i18n("Tourist Attraction")

    ColumnLayout {
        spacing: 0

        FormCard.FormHeader {
            title: touristAttraction.name
        }

        FormCard.FormCard {
            App.FormPlaceDelegate {
                id: departureDelegate
                place: touristAttraction
                controller: root.controller
            }
        }

        // arrival data
        FormCard.FormHeader {
            title: i18n("Departure")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                text: i18n("Departure Time")
                description: Localizer.formatDateTime(reservation, "departureTime")
            }
        }

        // arrival data
        FormCard.FormHeader {
            title: i18n("Arrival")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                text: i18n("Arrival Time")
                description: Localizer.formatDateTime(reservation, "arrivalTime")
            }
        }
    }
}

