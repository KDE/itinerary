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
    title: i18n("Flight")
    property var resIds: ReservationManager.reservationsForBatch(root.batchId)
    editor: App.FlightEditor {}

    function airportDisplayString(airport) {
        if (airport.name && airport.iataCode) {
            return airport.name + " (" + airport.iataCode + ")";
        } else {
            return airport.name || airport.iataCode || "";
        }
    }

    data: BarcodeScanModeButton {
        page: root
        visible: ticketToken.hasBarcode
    }

    ColumnLayout {
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                Kirigami.Heading {
                    Layout.fillWidth: true
                    Layout.topMargin: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing
                    text: reservationFor.airline.iataCode + " " + reservationFor.flightNumber
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

                // sequence number belongs to the selected barcode
                MobileForm.FormTextDelegate {
                    id: sequenceNumberDelegate
                    text: i18n("Sequence Number:")
                    description: ReservationManager.reservation(ticketToken.currentReservationId).passengerSequenceNumber
                    visible: description
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            visible: boardingTimeLabel.visible || boardingGroupLabel.visible || seatLabel.visible || airlineNameLabel.visible
            contentItem: ColumnLayout {
                spacing: 0

                // flight data
                MobileForm.FormCardHeader {
                    title: i18n("Boarding")
                }

                MobileForm.FormTextDelegate {
                    id: boardingTimeLabel
                    visible: reservationFor.boardingTime > 0
                    text: i18n("Boarding time")
                    description: Localizer.formatDateTime(reservationFor, "boardingTime")
                }

                MobileForm.FormDelegateSeparator { visible: boardingTimeLabel.visible }

                MobileForm.FormTextDelegate {
                    id: boardingGroupLabel
                    visible: reservation.boardingGroup.length > 0
                    text: i18n("Boarding group")
                    description: reservation.boardingGroup
                }

                MobileForm.FormDelegateSeparator { visible: boardingGroupLabel.visible }

                MobileForm.FormTextDelegate {
                    id: seatLabel
                    text: i18n("Seat")
                    description: reservation.airplaneSeat
                    visible: reservation.airplaneSeat.length > 0
                }

                MobileForm.FormDelegateSeparator { visible: seatLabel.visible }

                MobileForm.FormTextDelegate {
                    id: airlineNameLabel
                    text: i18n("Airline")
                    description: reservationFor.airline.name
                    visible: reservationFor.airline.name.length > 0
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
                    title: i18nc("Flight departure", "Departure")
                }

                MobileForm.FormTextDelegate {
                    id: departureTimeDelegate
                    text: i18n("Departure time")
                    description: Localizer.formatDateTime(reservationFor, "departureTime")
                    visible: reservationFor.departureTime > 0
                }
                MobileForm.FormTextDelegate {
                    text: i18n("Departure date")
                    visible: !departureTimeDelegate.visible && text.length > 0
                    description: Localizer.formatDate(reservationFor, "departureDay")
                }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormTextDelegate {
                    id: departureAirportDelegate
                    text: i18n("Airport")
                    description: airportDisplayString(reservationFor.departureAirport)
                    visible: text.length > 0
                }

                MobileForm.FormDelegateSeparator { visible: departureAirportDelegate.text }

                MobileForm.FormTextDelegate {
                    text: i18n("Terminal")
                    description: reservationFor.departureTerminal
                    visible: reservationFor.departureTerminal.length > 0
                }

                MobileForm.FormDelegateSeparator { visible: reservationFor.departureTerminal.length > 0 }

                MobileForm.FormTextDelegate {
                    text: i18n("Gate")
                    description: reservationFor.departureGate
                    visible: reservationFor.departureGate.length > 0
                }

                MobileForm.FormDelegateSeparator { visible: reservationFor.departureGate.length > 0 }

                App.FormPlaceDelegate {
                    place: reservationFor.departureAirport
                    controller: root.controller
                    isRangeBegin: true
                }
            }
        }

        // arrival data
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                // departure data
                MobileForm.FormCardHeader {
                    title: i18nc("Flight arrival", "Arrival")
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Arrival time")
                    description: Localizer.formatDateTime(reservationFor, "arrivalTime")
                    visible: reservationFor.arrivalTime > 0
                }

                MobileForm.FormDelegateSeparator { visible: reservationFor.arrivalTime > 0 }

                MobileForm.FormTextDelegate {
                    id: arrivalAirportDelegate
                    text: i18n("Airport")
                    description: airportDisplayString(reservationFor.arrivalAirport)
                    visible: text.length > 0
                }

                MobileForm.FormDelegateSeparator { visible: arrivalAirportDelegate.visible }

                MobileForm.FormTextDelegate {
                    text: i18n("Terminal")
                    description: reservationFor.arrivalTerminal
                    visible: reservationFor.arrivalTerminal.length > 0
                }

                MobileForm.FormDelegateSeparator { visible: reservationFor.arrivalTerminal.length > 0 }

                App.FormPlaceDelegate {
                    place: reservationFor.arrivalAirport
                    controller: root.controller
                    isRangeEnd: true
                }
            }
        }

        App.BookingCard {
            reservation: root.reservation
        }

        App.ProgramMembershipCard {
            programMembership: root.reservation.programMembershipUsed
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
