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
    title: i18nc("event as in concert/conference/show, not as in appointment", "Event")

    data: BarcodeScanModeButton {
        page: root
        visible: ticketToken.hasBarcode
    }

    ColumnLayout {
        spacing: 0

        CardPageTitle {
            emojiIcon: "🎤🎸🎶🏟"
            text: reservationFor.name
        }

        FormCard.FormCard {
            visible: ticketToken.ticketTokenCount > 0 || descriptionLabel.visible
            // ticket barcode
            TicketTokenDelegate {
                id: ticketToken
                Layout.fillWidth: true
                resIds: ReservationManager.reservationsForBatch(root.batchId)
                onCurrentReservationIdChanged: {
                    if (!currentReservationId)
                        return;
                    root.currentReservationId = currentReservationId;
                }
                onScanModeToggled: scanModeController.toggle()
                visible: ticketToken.ticketTokenCount > 0
            }

            FormCard.FormTextDelegate {
                id: descriptionLabel
                description: Util.textToHtml(reservationFor.description)
                visible: reservationFor.description
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }

        FormCard.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing

            FormPlaceDelegate {
                place: reservationFor.location
                controller: root.controller
                showLocationName: true
                visible: reservationFor.location != undefined
            }

            FormCard.FormDelegateSeparator { visible: reservationFor.location != undefined }

            FormCard.FormTextDelegate {
                text: i18nc("time of entrance", "Entrance")
                description: Localizer.formatDateTime(reservationFor, "doorTime")
                visible: reservationFor.doorTime > 0
            }

            FormCard.FormDelegateSeparator { visible: reservationFor.doorTime > 0 }

            FormCard.FormTextDelegate {
                text: i18n("Start Time")
                description: Localizer.formatDateTime(reservationFor, "startDate")
                visible: reservationFor.startDate > 0
            }

            FormCard.FormDelegateSeparator { visible: reservationFor.startDate > 0 }

            FormCard.FormTextDelegate {
                text: i18n("End Time")
                description: Localizer.formatDateTime(reservationFor, "endDate")
                visible: reservationFor.endDate > 0
            }

            FormCard.FormDelegateSeparator { visible: reservationFor.url != "" }
            FormCard.FormButtonDelegate {
                text: i18n("Website")
                description: reservationFor.url
                icon.name: "globe"
                onClicked: Qt.openUrlExternally(reservationFor.url)
                visible: reservationFor.url != ""
            }
        }

        // seat reservation
        FormCard.FormHeader {
            visible: seatSectionLabel.visible || seatRowLabel.visible || seatNumberLabel.visible
            title: i18n("Seat")
        }

        FormCard.FormCard {
            visible: seatSectionLabel.visible || seatRowLabel.visible || seatNumberLabel.visible || seatTypeLabel.visible
            FormCard.FormTextDelegate {
                id: seatSectionLabel
                text: i18nc("seat section, e.g. block in a stadium", "Section:")
                description: root.reservation.reservedTicket.ticketedSeat.seatSection
                visible: description
            }
            FormCard.FormDelegateSeparator {
                visible: root.reservation.reservedTicket.ticketedSeat.seatSection
            }
            FormCard.FormTextDelegate {
                id: seatRowLabel
                text: i18nc("seat row", "Row:")
                description: root.reservation.reservedTicket.ticketedSeat.seatRow
                visible: description
            }
            FormCard.FormDelegateSeparator {
                visible: root.reservation.reservedTicket.ticketedSeat.seatRow
            }
            FormCard.FormTextDelegate {
                id: seatNumberLabel
                text: i18nc("seat number", "Number:")
                description: root.reservation.reservedTicket.ticketedSeat.seatNumber
                visible: description
            }
            FormCard.FormDelegateSeparator {
                visible: root.reservation.reservedTicket.ticketedSeat.seatNumber
            }
            FormCard.FormTextDelegate {
                id: seatTypeLabel
                text: i18nc("seat type", "Type:")
                description: root.reservation.reservedTicket.ticketedSeat.seatingType
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
            batchId: root.batchId
            editor: root.editor
            reservation: root.reservation
        }
    }
}
