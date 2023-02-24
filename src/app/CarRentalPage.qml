// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.DetailsPage {
    id: root
    title: i18n("Rental Car")

    ColumnLayout {
        width: parent.width
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18n("Pick-up")
                }

                MobileForm.FormTextDelegate {
                    id: pickupTimeDelegate
                    text: i18n("Time")
                    description: Localizer.formatDateTime(reservation, "pickupTime")
                }

                MobileForm.FormDelegateSeparator {
                    visible: pickupTimeDelegate.description
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Location")
                    description: reservation.pickupLocation.name
                }

                App.FormPlaceDelegate {
                    place: reservation.pickupLocation
                    controller: root.controller
                    isRangeBegin: true
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18n("Drop-off")
                }

                MobileForm.FormTextDelegate {
                    id: dropoffTimeDelegate
                    text: i18n("Time")
                    description: Localizer.formatDateTime(reservation, "dropoffTime")
                }

                MobileForm.FormDelegateSeparator {
                    visible: dropoffTimeDelegate.description
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Location")
                    description: reservation.dropoffLocation.name
                }

                App.FormPlaceDelegate {
                    place: reservation.dropoffLocation
                    controller: root.controller
                    isRangeEnd: true
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18n("Vehicle")
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Type")
                    description: reservationFor.name
                    visible: description
                }

                MobileForm.FormDelegateSeparator {
                    visible: reservationFor.name
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Model")
                    description: reservationFor.model
                    visible: description
                }

                MobileForm.FormDelegateSeparator {
                    visible: reservationFor.model
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Brand")
                    description: reservationFor.brand.name
                    visible: description
                }

                MobileForm.FormDelegateSeparator {
                    visible: reservationFor.brand.name
                }
            }
        }

        App.BookingCard {
            currentReservation: root.currentReservation
            reservation: root.reservation
        }

        App.DocumentsPage {
            controller: root.controller
        }

        App.ActionsCard {
            batchId: root.batchId
            editor: root.editor
        }
    }
}

