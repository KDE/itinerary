// SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
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

    title: i18n("Boat Ticket")

    data: BarcodeScanModeButton {
        page: root
        visible: ticketToken.hasBarcode
    }

    ColumnLayout {
        spacing: 0

        CardPageTitle {
            emojiIcon: "🛳️"
            text: i18n("Boat")

            // TODO vessel name not yet available in the data model
            // text: reservationFor.boatName
        }

        FormCard.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            visible: ticketToken.ticketTokenCount > 0

            // ticket barcode
            TicketTokenDelegate {
                id: ticketToken
                Layout.topMargin: Kirigami.Units.largeSpacing
                resIds: ReservationManager.reservationsForBatch(root.batchId)
                onCurrentReservationIdChanged: {
                    if (!currentReservationId)
                        return;
                    root.currentReservationId = currentReservationId;
                }
                onScanModeToggled: scanModeController.toggle()
            }
        }

        // departure data
        FormCard.FormHeader {
            title: i18nc("Boat departure", "Departure")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                text: i18n("Departure time")
                description: Localizer.formatDateTime(reservationFor, "departureTime")
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                text: i18nc("Boat terminal", "Terminal")
                description: reservationFor.departureBoatTerminal.name
            }

            FormCard.FormDelegateSeparator {
                visible: departurePlace
            }

            FormPlaceDelegate {
                id: departurePlace
                place: reservationFor.departureBoatTerminal
                controller: root.controller
                isRangeBegin: true
            }
        }

        // arrival data
        FormCard.FormHeader {
            title: i18nc("Boat arrival", "Arrival")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                text: i18n("Arrival time")
                description: Localizer.formatDateTime(reservationFor, "arrivalTime")
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                text: i18nc("Boat terminal", "Terminal")
                description: reservationFor.arrivalBoatTerminal.name
            }

            FormCard.FormDelegateSeparator {
                visible: arrivalPlace
            }

            FormPlaceDelegate {
                id: arrivalPlace
                place: reservationFor.arrivalBoatTerminal
                controller: root.controller
                isRangeEnd: true
            }
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

