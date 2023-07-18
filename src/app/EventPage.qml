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
    title: i18nc("event as in concert/conference/show, not as in appointment", "Event")
    editor: App.EventEditor {}

    @KIRIGAMI_MAIN_ACTIONS@: Kirigami.Action {
        icon.name: "view-barcode-qr"
        text: i18n("Barcode Scan Mode")
        onTriggered: scanModeController.toggle()
        visible: ticketToken.hasBarcode
        checkable: true
        checked: scanModeController.enabled
    }

    BarcodeScanModeController {
        id: scanModeController
        page: root
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

                // ticket barcode
                App.TicketTokenDelegate {
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

                MobileForm.FormTextDelegate {
                    description: Util.textToHtml(reservationFor.description)
                    visible: reservationFor.description
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                App.FormPlaceDelegate {
                    place: reservationFor.location
                    controller: root.controller
                    showLocationName: true
                    visible: reservationFor.location != undefined
                }

                MobileForm.FormDelegateSeparator { visible: reservationFor.location != undefined }

                MobileForm.FormTextDelegate {
                    text: i18nc("time of entrance", "Entrance")
                    description: Localizer.formatDateTime(reservationFor, "doorTime")
                    visible: reservationFor.doorTime > 0
                }

                MobileForm.FormDelegateSeparator { visible: reservationFor.doorTime > 0 }

                MobileForm.FormTextDelegate {
                    text: i18n("Start Time")
                    description: Localizer.formatDateTime(reservationFor, "startDate")
                    visible: reservationFor.startDate > 0
                }

                MobileForm.FormDelegateSeparator { visible: reservationFor.startDate > 0 }

                MobileForm.FormTextDelegate {
                    text: i18n("End Time")
                    description: Localizer.formatDateTime(reservationFor, "endDate")
                    visible: reservationFor.endDate > 0
                }

                MobileForm.FormDelegateSeparator { visible: reservationFor.url != "" }
                MobileForm.FormButtonDelegate {
                    text: i18n("Website")
                    description: reservationFor.url
                    icon.name: "globe"
                    onClicked: Qt.openUrlExternally(reservationFor.url)
                    visible: reservationFor.url != ""
                }
            }
        }

        // seat reservation
        MobileForm.FormCard {
            visible: seatSectionLabel.visible || seatRowLabel.visible || seatNumberLabel.visible
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                MobileForm.FormCardHeader {
                    title: i18n("Seat")
                }
                MobileForm.FormTextDelegate {
                    id: seatSectionLabel
                    text: i18nc("seat section, e.g. block in a stadium", "Section:")
                    description: root.reservation.reservedTicket.ticketedSeat.seatSection
                    visible: description
                }
                MobileForm.FormDelegateSeparator {
                    visible: root.reservation.reservedTicket.ticketedSeat.seatSection
                }
                MobileForm.FormTextDelegate {
                    id: seatRowLabel
                    text: i18nc("seat row", "Row:")
                    description: root.reservation.reservedTicket.ticketedSeat.seatRow
                    visible: description
                }
                MobileForm.FormDelegateSeparator {
                    visible: root.reservation.reservedTicket.ticketedSeat.seatRow
                }
                MobileForm.FormTextDelegate {
                    id: seatNumberLabel
                    text: i18nc("seat number", "Number:")
                    description: root.reservation.reservedTicket.ticketedSeat.seatNumber
                    visible: description
                }
            }
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
