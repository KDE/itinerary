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

ScrollableDetailsPage {
    id: root
    title: i18n("Rental Car")

    ColumnLayout {
        spacing: 0

        CardPageTitle {
            emojiIcon: "🚗"
            text: i18n("Rental Car")
        }

        FormCard.FormHeader {
            title: i18n("Pick-up")
        }

        FormCard.FormCard {
            autoSeparators: true

            FormCard.FormTextDelegate {
                id: pickupTimeDelegate
                text: i18n("Time")
                description: Localizer.formatDateTime(reservation, "pickupTime")
                visible: description !== ""
            }

            FormPlaceDelegate {
                id: departureLocation

                text: i18n("Location")
                placeName: reservation.pickupLocation.name
                place: reservation.pickupLocation
                controller: root.controller
                isRangeBegin: true
            }
        }

        FormCard.FormHeader {
            title: i18n("Drop-off")
        }

        FormCard.FormCard {
            autoSeparators: true

            FormCard.FormTextDelegate {
                id: dropoffTimeDelegate
                text: i18n("Time")
                description: Localizer.formatDateTime(reservation, "dropoffTime")
                visible: description !== ""
            }

            FormPlaceDelegate {
                id: arrivalLocation

                text: i18n("Location")
                placeName: reservation.dropoffLocation.name
                place: reservation.dropoffLocation
                controller: root.controller
                isRangeEnd: true
            }
        }

        FormCard.FormHeader {
            title: i18n("Vehicle")
        }

        FormCard.FormCard {
            autoSeparators: true

            FormCard.FormTextDelegate {
                text: i18n("Type")
                description: reservationFor.name
                visible: description
            }

            FormCard.FormTextDelegate {
                text: i18n("Model")
                description: reservationFor.model
                visible: description
            }

            FormCard.FormTextDelegate {
                text: i18n("Brand")
                description: reservationFor.brand.name
                visible: description
            }
        }

        BookingCard {
            reservation: root.reservation
        }

        ReservationDocumentsCard {
            controller: root.controller
        }

        ActionsCard {
            controller: root.controller
            batchId: root.batchId
            reservationId: root.currentReservationId
            editor: root.editor
            reservation: root.reservation
        }
    }
}

