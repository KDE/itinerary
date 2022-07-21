/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.DetailsPage {
    id: root
    title: i18n("Boat Ticket")

    actions.main: Kirigami.Action {
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

        // TODO vessel name not yet available in the data model
        /*QQC2.Label {
            Layout.fillWidth: true
            text: reservationFor.boatName
            horizontalAlignment: Qt.AlignHCenter
            font.bold: true
        }*/

        // ticket barcode
        App.TicketTokenDelegate {
            id: ticketToken
            resIds: ReservationManager.reservationsForBatch(root.batchId)
            onCurrentReservationIdChanged: {
                if (!currentReservationId)
                    return;
                root.currentReservationId = currentReservationId;
            }
            onScanModeToggled: scanModeController.toggle()
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true

            // departure data
            Kirigami.Separator {
                Kirigami.FormData.isSection: true
                Kirigami.FormData.label: i18n("Departure")
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Time:")
                text: Localizer.formatDateTime(reservationFor, "departureTime")
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Terminal:")
                text: reservationFor.departureBoatTerminal.name
            }
            App.PlaceDelegate {
                place: reservationFor.departureBoatTerminal
                controller: root.controller
                isRangeBegin: true
            }

            // arrival data
            Kirigami.Separator {
                Kirigami.FormData.label: i18n("Arrival")
                Kirigami.FormData.isSection: true
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Time:")
                text: Localizer.formatDateTime(reservationFor, "arrivalTime")
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Terminal:")
                text: reservationFor.arrivalBoatTerminal.name
            }
            App.PlaceDelegate {
                place: reservationFor.arrivalBoatTerminal
                controller: root.controller
                isRangeEnd: true
            }

            // booking details
            Kirigami.Separator {
                Kirigami.FormData.label: i18n("Booking")
                Kirigami.FormData.isSection: true
                visible: referenceLabel.visible || underNameLabel.visible
            }
            QQC2.Label {
                id: referenceLabel
                Kirigami.FormData.label: i18n("Reference:")
                text: reservation.reservationNumber
                visible: reservation.reservationNumber
            }
            QQC2.Label {
                id: underNameLabel
                Kirigami.FormData.label: i18n("Under name:")
                text: reservation.underName.name
                visible: reservation.underName.name !== ""
            }
        }
    }
}

