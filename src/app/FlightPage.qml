// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
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
        spacing: 0

        App.CardPageTitle {
            emojiIcon: "✈️"
            text: reservationFor.airline.iataCode + " " + reservationFor.flightNumber
        }

        FormCard.FormCard {
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

            FormCard.FormDelegateSeparator {}

            // sequence number belongs to the selected barcode
            FormCard.FormTextDelegate {
                id: sequenceNumberDelegate
                text: i18n("Sequence Number:")
                description: ReservationManager.reservation(ticketToken.currentReservationId).passengerSequenceNumber
                visible: description
            }
        }

        // flight data
        FormCard.FormHeader {
            visible: boardingTimeLabel.visible || boardingGroupLabel.visible || seatLabel.visible || airlineNameLabel.visible
            title: i18n("Boarding")
        }

        FormCard.FormCard {
            visible: boardingTimeLabel.visible || boardingGroupLabel.visible || seatLabel.visible || airlineNameLabel.visible

            FormCard.FormTextDelegate {
                id: boardingTimeLabel
                visible: reservationFor.boardingTime > 0
                text: i18n("Boarding time")
                description: Localizer.formatDateTime(reservationFor, "boardingTime")
            }

            FormCard.FormDelegateSeparator { visible: boardingTimeLabel.visible }

            FormCard.FormTextDelegate {
                id: boardingGroupLabel
                visible: reservation.boardingGroup.length > 0
                text: i18n("Boarding group")
                description: reservation.boardingGroup
            }

            FormCard.FormDelegateSeparator { visible: boardingGroupLabel.visible }

            FormCard.FormTextDelegate {
                id: seatLabel
                text: i18n("Seat")
                description: reservation.airplaneSeat
                visible: reservation.airplaneSeat.length > 0
            }

            FormCard.FormDelegateSeparator { visible: seatLabel.visible }

            FormCard.FormTextDelegate {
                id: airlineNameLabel
                text: i18n("Airline")
                description: reservationFor.airline.name
                visible: reservationFor.airline.name.length > 0
            }
        }

        // departure data
        FormCard.FormHeader {
            title: i18nc("Flight departure", "Departure")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                id: departureTimeDelegate
                text: i18n("Departure time")
                description: Localizer.formatDateTime(reservationFor, "departureTime")
                visible: reservationFor.departureTime > 0
            }
            FormCard.FormTextDelegate {
                text: i18n("Departure date")
                visible: !departureTimeDelegate.visible && text.length > 0
                description: Localizer.formatDate(reservationFor, "departureDay")
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                id: departureAirportDelegate
                text: i18n("Airport")
                description: airportDisplayString(reservationFor.departureAirport)
                visible: text.length > 0
            }

            FormCard.FormDelegateSeparator { visible: departureAirportDelegate.text }

            FormCard.FormTextDelegate {
                text: i18n("Terminal")
                description: reservationFor.departureTerminal
                visible: reservationFor.departureTerminal.length > 0
            }

            FormCard.FormDelegateSeparator { visible: reservationFor.departureTerminal.length > 0 }

            FormCard.FormTextDelegate {
                text: i18n("Gate")
                description: reservationFor.departureGate
                visible: reservationFor.departureGate.length > 0
            }

            FormCard.FormDelegateSeparator { visible: reservationFor.departureGate.length > 0 }

            App.FormPlaceDelegate {
                place: reservationFor.departureAirport
                controller: root.controller
                isRangeBegin: true
            }
        }

        // arrival data
        FormCard.FormHeader {
            title: i18nc("Flight arrival", "Arrival")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                text: i18n("Arrival time")
                description: Localizer.formatDateTime(reservationFor, "arrivalTime")
                visible: reservationFor.arrivalTime > 0
            }

            FormCard.FormDelegateSeparator { visible: reservationFor.arrivalTime > 0 }

            FormCard.FormTextDelegate {
                id: arrivalAirportDelegate
                text: i18n("Airport")
                description: airportDisplayString(reservationFor.arrivalAirport)
                visible: text.length > 0
            }

            FormCard.FormDelegateSeparator { visible: arrivalAirportDelegate.visible }

            FormCard.FormTextDelegate {
                text: i18n("Terminal")
                description: reservationFor.arrivalTerminal
                visible: reservationFor.arrivalTerminal.length > 0
            }

            FormCard.FormDelegateSeparator { visible: reservationFor.arrivalTerminal.length > 0 }

            App.FormPlaceDelegate {
                place: reservationFor.arrivalAirport
                controller: root.controller
                isRangeEnd: true
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
