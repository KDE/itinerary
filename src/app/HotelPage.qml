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

    title: i18n("Hotel Reservation")

    ColumnLayout {
        CardPageTitle {
            emojiIcon: "🏨"
            text: reservationFor.name
        }

        FormCard.FormHeader {
            title: i18n("Location")
        }

        FormCard.FormCard {
            FormPlaceDelegate {
                place: reservationFor
                controller: root.controller
            }
        }

        ContactCard {
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

        ProgramMembershipCard {
            programMembership: root.reservation.programMembershipUsed
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
