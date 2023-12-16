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
    title: i18n("Rental Car")

    ColumnLayout {
        spacing: 0

        App.CardPageTitle {
            emojiIcon: "🚗"
            text: i18n("Rental Car")
        }

        FormCard.FormHeader {
            title: i18n("Pick-up")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                id: pickupTimeDelegate
                text: i18n("Time")
                description: Localizer.formatDateTime(reservation, "pickupTime")
            }

            FormCard.FormDelegateSeparator {
                visible: pickupTimeDelegate.description
            }

            FormCard.FormTextDelegate {
                text: i18n("Location")
                description: reservation.pickupLocation.name
            }

            App.FormPlaceDelegate {
                place: reservation.pickupLocation
                controller: root.controller
                isRangeBegin: true
            }
        }

        FormCard.FormHeader {
            title: i18n("Drop-off")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                id: dropoffTimeDelegate
                text: i18n("Time")
                description: Localizer.formatDateTime(reservation, "dropoffTime")
            }

            FormCard.FormDelegateSeparator {
                visible: dropoffTimeDelegate.description
            }

            FormCard.FormTextDelegate {
                text: i18n("Location")
                description: reservation.dropoffLocation.name
            }

            App.FormPlaceDelegate {
                place: reservation.dropoffLocation
                controller: root.controller
                isRangeEnd: true
            }
        }

        FormCard.FormHeader {
            title: i18n("Vehicle")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                text: i18n("Type")
                description: reservationFor.name
                visible: description
            }

            FormCard.FormDelegateSeparator {
                visible: reservationFor.name
            }

            FormCard.FormTextDelegate {
                text: i18n("Model")
                description: reservationFor.model
                visible: description
            }

            FormCard.FormDelegateSeparator {
                visible: reservationFor.model
            }

            FormCard.FormTextDelegate {
                text: i18n("Brand")
                description: reservationFor.brand.name
                visible: description
            }

            FormCard.FormDelegateSeparator {
                visible: reservationFor.brand.name
            }
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

