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
    title: i18n("Restaurant Reservation")
    editor: restaurantEditorPage

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
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
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
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormTextDelegate {
                    text: i18n("Start time")
                    description: Localizer.formatDateTime(reservation, "startTime")
                    visible: description
                }

                MobileForm.FormDelegateSeparator { visible: reservation.startTime > 0 }

                MobileForm.FormTextDelegate {
                    text: i18n("End time")
                    description: Localizer.formatDateTime(reservation, "endTime")
                    visible: description
                }

                MobileForm.FormDelegateSeparator { visible: reservation.endTime > 0 }

                MobileForm.FormTextDelegate {
                    text: i18n("Party size")
                    description: reservation.partySize
                    visible: reservation.partySize > 0
                }
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
            passId: root.passId
        }
    }
}
