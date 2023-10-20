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

    title: i18n("Hotel Reservation")

    ColumnLayout {
        App.CardPageTitle {
            emojiIcon: "🏨"
            text: reservationFor.name
        }

        FormCard.FormHeader {
            title: i18n("Location")
        }

        FormCard.FormCard {
            App.FormPlaceDelegate {
                place: reservationFor
                controller: root.controller
            }
        }

        App.ContactCard {
            contact: root.reservationFor
        }

        FormCard.FormCard {
            visible: reservation.checkinTime > 0 || reservation.checkoutTime > 0
            FormCard.FormTextDelegate {
                text: i18n("Check-in time")
                description: Util.isStartOfDay(reservation, "checkinTime") ? Localizer.formatDate(reservation, "checkinTime") : Localizer.formatDateTime(reservation, "checkinTime")
            }

            FormCard.FormDelegateSeparator { visible: reservation.checkinTime > 0 }

            FormCard.FormTextDelegate {
                text: i18n("Check-out time")
                description: Util.isStartOfDay(reservation, "checkoutTime") ? Localizer.formatDate(reservation, "checkoutTime") : Localizer.formatDateTime(reservation, "checkoutTime")
            }
        }

        App.ProgramMembershipCard {
            programMembership: root.reservation.programMembershipUsed
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
