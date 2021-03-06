/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.DetailsPage {
    id: root
    title: i18n("Flight")
    property var resIds: ReservationManager.reservationsForBatch(root.batchId)
    editor: Component {
        App.FlightEditor {
            batchId: root.batchId
        }
    }

    function airportDisplayString(airport) {
        if (airport.name && airport.iataCode) {
            return airport.name + " (" + airport.iataCode + ")";
        } else {
            return airport.name || airport.iataCode || "";
        }
    }

    ColumnLayout {
        width: parent.width

        QQC2.Label {
            Layout.fillWidth: true
            text: reservationFor.airline.iataCode + " " + reservationFor.flightNumber
            horizontalAlignment: Qt.AlignHCenter
            font.bold: true
        }

        // ticket barcode
        App.TicketTokenDelegate {
            id: ticketDelegate
            resIds: ReservationManager.reservationsForBatch(root.batchId)
            onCurrentReservationIdChanged: {
                if (!currentReservationId)
                    return;
                root.currentReservationId = currentReservationId;
            }
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true

            // sequence number belongs to the selected barcode
            QQC2.Label {
                Kirigami.FormData.label: i18n("Sequence Number:")
                text: ReservationManager.reservation(ticketDelegate.currentReservationId).passengerSequenceNumber
                visible: text
            }

            // flight details
            Kirigami.Separator {
                Kirigami.FormData.isSection: true
                Kirigami.FormData.label: i18n("Flight")
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Boarding:")
                text: Localizer.formatDateTime(reservationFor, "boardingTime")
                visible: reservationFor.boardingTime > 0

            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Boarding group:")
                text: reservation.boardingGroup
                visible: reservation.boardingGroup.length > 0

            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Seat:")
                text: reservation.airplaneSeat
                visible: reservation.airplaneSeat.length > 0

            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Airline:")
                text: reservationFor.airline.name
                visible: reservationFor.airline.name.length > 0
            }


            // departure data
            Kirigami.Separator {
                Kirigami.FormData.isSection: true
                Kirigami.FormData.label: i18nc("flight departure", "Departure")
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Time:")
                text: Localizer.formatDateTime(reservationFor, "departureTime")
                visible: reservationFor.departureTime > 0
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Airport:")
                text: airportDisplayString(reservationFor.departureAirport)
                visible: text.length > 0
            }
            App.PlaceDelegate {
                place: reservationFor.departureAirport
                controller: root.controller
                isRangeBegin: true
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Terminal:")
                text: reservationFor.departureTerminal
                visible: reservationFor.departureTerminal.length > 0
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Gate:")
                text: reservationFor.departureGate
                visible: reservationFor.departureGate.length > 0
            }


            // arrival data
            Kirigami.Separator {
                Kirigami.FormData.isSection: true
                Kirigami.FormData.label: i18nc("flight arrival", "Arrival")
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Time:")
                text: Localizer.formatDateTime(reservationFor, "arrivalTime")
                visible: reservationFor.arrivalTime > 0
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Airport:")
                text: airportDisplayString(reservationFor.arrivalAirport)
                visible: text.length > 0
            }
            App.PlaceDelegate {
                place: reservationFor.arrivalAirport
                controller: root.controller
                isRangeEnd: true
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Terminal:")
                text: reservationFor.arrivalTerminal
                visible: reservationFor.arrivalTerminal.length > 0
            }

            // booking details
            Kirigami.Separator {
                Kirigami.FormData.isSection: true
                Kirigami.FormData.label: i18n("Booking")
            }
            Repeater {
                model: resIds.length * 2
                delegate: QQC2.Label {
                    property var res: ReservationManager.reservation(resIds[Math.floor(index/2)]);
                    Kirigami.FormData.label: index % 2 == 0 ? i18n("Under name:") : i18n("Reference:")
                    text: index % 2 == 0 ? res.underName.name : res.reservationNumber
                }
            }
        }
    }
}
