/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import "." as App

App.DetailsPage {
    id: root
    title: i18n("Bus Ticket")

    Component {
        id: alternativePage
        App.AlternativeJourneyPage {
            controller: root.controller
            publicTransportManager: LiveDataManager.publicTransportManager
        }
    }

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
                Kirigami.Heading {
                    Layout.fillWidth: true
                    Layout.topMargin: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing
                    text: reservationFor.busName + " " + reservationFor.busNumber
                    horizontalAlignment: Qt.AlignHCenter
                    font.bold: true
                }

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

                // departure data
                MobileForm.FormCardHeader {
                    title: i18n("Departure")
                }

                MobileForm.FormTextDelegate {
                    visible: departureTimeLabel.length > 0
                    contentItem: ColumnLayout {
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing
                        QQC2.Label {
                            Layout.fillWidth: true
                            text: i18n("Departure time")
                            elide: Text.ElideRight
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            QQC2.Label {
                                id: departureTimeLabel
                                text: Localizer.formatDateTime(reservationFor, "departureTime")
                                color: Kirigami.Theme.disabledTextColor
                                font: Kirigami.Theme.smallFont
                                elide: Text.ElideRight
                            }
                            QQC2.Label {
                                text: (departure.departureDelay >= 0 ? "+" : "") + departure.departureDelay
                                color: (departure.departureDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                                visible: departure.hasExpectedDepartureTime
                            }
                        }
                    }
                }
                MobileForm.FormTextDelegate {
                    text: i18nc("bus station", "Station")
                    description: reservationFor.departureBusStop.name
                }

                MobileForm.FormTextDelegate {
                    Layout.fillWidth: true
                    contentItem: App.PlaceDelegate {
                        place: reservationFor.departureBusStop
                        controller: root.controller
                        isRangeBegin: true
                    }
                }

                // arrival data
                MobileForm.FormCardHeader {
                    title: i18n("Arrival")
                }

                MobileForm.FormTextDelegate {
                    contentItem: ColumnLayout {
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing
                        QQC2.Label {
                            Layout.fillWidth: true
                            text: i18n("Arrival time")
                            elide: Text.ElideRight
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            QQC2.Label {
                                text: Localizer.formatDateTime(reservationFor, "arrivalTime")
                                color: Kirigami.Theme.disabledTextColor
                                font: Kirigami.Theme.smallFont
                                elide: Text.ElideRight
                            }
                            QQC2.Label {
                                font: Kirigami.Theme.smallFont
                                text: (arrival.arrivalDelay >= 0 ? "+" : "") + arrival.arrivalDelay
                                color: (arrival.arrivalDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                                visible: arrival.hasExpectedArrivalTime
                            }
                        }
                    }
                }
                MobileForm.FormTextDelegate {
                    text: i18nc("bus station", "Station")
                    description: reservationFor.arrivalBusStop.name
                }

                MobileForm.FormTextDelegate {
                    Layout.fillWidth: true
                    contentItem: App.PlaceDelegate {
                        place: reservationFor.arrivalBusStop
                        controller: root.controller
                        isRangeBegin: true
                    }
                }
            }
        }

        // seat reservation
        MobileForm.FormCard {
            visible: seatLabel.visible
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                MobileForm.FormCardHeader {
                    title: i18n("Seat")
                }
                MobileForm.FormTextDelegate {
                    id: seatLabel
                    Kirigami.FormData.label: i18n("Seat")
                    text: reservation.reservedTicket.ticketedSeat.seatNumber
                    visible: text !== ""
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
            additionalActions: [
                QQC2.Action {
                    text: i18n("Alternatives")
                    icon.name: "clock"
                    onTriggered: applicationWindow().pageStack.push(alternativePage)
                },
                Kirigami.Action {
                    text: i18n("Journey Details")
                    icon.name: "view-calendar-day"
                    onTriggered: applicationWindow().pageStack.push(journeySectionPage, {"journeySection": root.controller.journey});
                    visible: root.controller.journey && root.controller.journey.intermediateStops.length > 0
                }
            ]
        }
    }
}
