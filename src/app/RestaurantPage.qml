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

    title: i18n("Restaurant Reservation")

    ColumnLayout {
        spacing: 0

        App.CardPageTitle {
            emojiIcon: "🍽️"
            text: reservationFor.name

            Layout.fillWidth: true
        }

        FormCard.FormHeader {
            title: i18nc("@title:group", "Details")
        }

        FormCard.FormCard {
            App.FormPlaceDelegate {
                place: reservationFor
                controller: root.controller
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                text: i18n("Start time")
                description: Localizer.formatDateTime(reservation, "startTime")
                visible: description
            }

            FormCard.FormDelegateSeparator { visible: reservation.startTime > 0 }

            FormCard.FormTextDelegate {
                text: i18n("End time")
                description: Localizer.formatDateTime(reservation, "endTime")
                visible: description
            }

            FormCard.FormDelegateSeparator { visible: reservation.endTime > 0 }

            FormCard.FormTextDelegate {
                text: i18n("Party size")
                description: reservation.partySize
                visible: reservation.partySize > 0
            }
        }

        App.ContactCard {
            contact: root.reservationFor
        }

        App.BookingCard {
            reservation: root.reservation
        }

        App.ReservationDocumentsCard {
            controller: root.controller
        }

        App.ActionsCard {
            batchId: root.batchId
            editor: root.editor
            reservation: root.reservation
        }
    }
}
