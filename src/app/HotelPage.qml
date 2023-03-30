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
    title: i18n("Hotel Reservation")
    editor: App.HotelEditor {
        batchId: root.batchId
    }

    ColumnLayout {
        width: parent.width
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                Kirigami.Heading {
                    Layout.fillWidth: true
                    Layout.topMargin: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing
                    text: reservationFor.name
                    horizontalAlignment: Qt.AlignHCenter
                    font.bold: true
                    wrapMode: Text.WordWrap
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Location")
                }

                App.FormPlaceDelegate {
                    place: reservationFor
                    controller: root.controller
                }
            }
        }

        App.ContactCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contact: root.reservationFor
        }

        MobileForm.FormCard {
            visible: reservation.checkinTime > 0 || reservation.checkoutTime > 0
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormTextDelegate {
                    text: i18n("Check-in time")
                    description: Localizer.formatDateTime(reservation, "checkinTime")
                }

                MobileForm.FormDelegateSeparator { visible: reservation.checkinTime > 0 }

                MobileForm.FormTextDelegate {
                    text: i18n("Check-out time")
                    description: Localizer.formatDateTime(reservation, "checkoutTime")
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
