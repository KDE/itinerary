// SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
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

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                // TODO vessel name not yet available in the data model
                /* Kirigami.Heading {
                    Layout.fillWidth: true
                    Layout.topMargin: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing
                    Layout.fillWidth: true
                    text: reservationFor.boatName
                    horizontalAlignment: Qt.AlignHCenter
                    font.bold: true
                }*/

                // ticket barcode
                App.TicketTokenDelegate {
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
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                // departure data
                MobileForm.FormCardHeader {
                    title: i18n("Departure")
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Departure time")
                    description: Localizer.formatDateTime(reservationFor, "departureTime")
                }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormTextDelegate {
                    text: i18nc("Boat terminal", "Terminal")
                    description: reservationFor.departureBoatTerminal.name
                }

                MobileForm.FormDelegateSeparator {
                    visible: departurePlace
                }

                App.FormPlaceDelegate {
                    id: departurePlace
                    place: reservationFor.departureBoatTerminal
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

                // arrival data
                MobileForm.FormCardHeader {
                    title: i18n("Arrival")
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Arrival time")
                    description: Localizer.formatDateTime(reservationFor, "arrivalTime")
                }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormTextDelegate {
                    text: i18nc("Boat terminal", "Terminal")
                    description: reservationFor.arrivalBoatTerminal.name
                }

                MobileForm.FormDelegateSeparator {
                    visible: arrivalPlace
                }

                App.FormPlaceDelegate {
                    id: arrivalPlace
                    place: reservationFor.arrivalBoatTerminal
                    controller: root.controller
                    isRangeEnd: true
                }
            }
        }

        App.BookingCard {
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

