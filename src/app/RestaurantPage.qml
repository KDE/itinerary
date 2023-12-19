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

DetailsPage {
    id: root

    title: i18n("Restaurant Reservation")

    ColumnLayout {
        spacing: 0

        CardPageTitle {
            emojiIcon: "🍽️"
            text: reservationFor.name

            Layout.fillWidth: true
        }

        FormCard.FormHeader {
            title: i18nc("@title:group", "Details")
        }

        FormCard.FormCard {
            FormPlaceDelegate {
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

        ContactCard {
            contact: root.reservationFor
        }

        BookingCard {
            reservation: root.reservation
        }

        ReservationDocumentsCard {
            controller: root.controller
        }

        ActionsCard {
            batchId: root.batchId
            editor: root.editor
            reservation: root.reservation
        }
    }
}
