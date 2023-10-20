// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
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
                text: i18n("Depature Time")
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

