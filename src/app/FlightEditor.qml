/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitinerary
import org.kde.itinerary

EditorPage {
    id: root
    title: root.isNew ? i18n("New Flight") : i18n("Edit Flight")

    isValidInput: departureTime.hasValue
        && (!arrivalTime.hasValue || departureTime.value < arrivalTime.value)
        && (root.reservation.reservationFor.departureAirport.iataCode !== "" || departureAirport.text !== "")
        && (root.reservation.reservationFor.arrivalAirport.iataCode !== "" || arrivalAirport.text !== "")
    tripGroupSelector: tripGroupSelector

    function apply(reservation) {
        var flight = reservation.reservationFor;
        if (boardingTime.isModified)
            flight = Util.setDateTimePreserveTimezone(flight, "boardingTime", boardingTime.value);

        let airport = flight.departureAirport;
        airport.name = departureAirportName.text;
        flight.departureAirport = airport;
        flight.departureGate = departureGate.text;
        flight.departureTerminal = departureTerminal.text;
        if (departureTime.isModified)
            flight = Util.setDateTimePreserveTimezone(flight, "departureTime", departureTime.value);

        airport = flight.arrivalAirport;
        airport.name = arrivalAirportName.text;
        flight.arrivalAirport = airport;
        flight.arrivalTerminal = arrivalTerminal.text;
        if (arrivalTime.isModified)
            flight = Util.setDateTimePreserveTimezone(flight, "arrivalTime", arrivalTime.value);

        const flightNum = flightNumber.text.match(/^\s*([A-Z0-9]{2})\s*(\d{1,4})\s*$/);
        if (flightNum) {
            let airline = flight.airline;
            airline.iataCode = flightNum[1];
            flight.airline = airline;
            flight.flightNumber = flightNum[2];
        }

        var newRes = reservation;
        newRes.airplaneSeat = seat.text;
        newRes.reservationFor = flight;
        bookingEdit.apply(newRes);
        return newRes;
    }

    ColumnLayout {
        spacing: 0

        CardPageTitle {
            emojiIcon: "✈️"
            text: reservation.reservationFor.airline.iataCode + " " + reservation.reservationFor.flightNumber

            Layout.fillWidth: true
        }

        FormCard.FormHeader {
            title: i18n("Trip")
            visible: root.isNew && Settings.developmentMode
        }

        TripGroupSelectorCard {
            id: tripGroupSelector
            visible: root.isNew && Settings.developmentMode
            suggestedName: arrivalAirportName.text
            tripGroupCandidates: TripGroupModel.intersectingXorAdjacentTripGroups(departureTime.value, arrivalTime.hasValue ? arrivalTime.value : departureTime.value)
        }

        FormCard.FormHeader {
            title: i18nc("flight departure", "Departure - %1", reservation.reservationFor.departureAirport.iataCode)
        }

        FormCard.FormCard {
            FormCard.FormTextFieldDelegate {
                id: departureAirportName
                label: i18n("Airport")
                text: reservation.reservationFor.departureAirport.name
                status: Kirigami.MessageType.Error
                statusMessage: reservation.reservationFor.departureAirport.iataCode === "" && departureAirportName.text === "" ? i18n("Departure airport has to be specified.") : ""
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormTextFieldDelegate {
                id: departureTerminal
                text: reservation.reservationFor.departureTerminal
                label: i18nc("flight departure terminal", "Terminal")
            }
            FormCard.FormDelegateSeparator {}
            FormDateTimeEditDelegate {
                id: departureTime
                text: i18nc("flight departure time", "Time")
                obj: reservation.reservationFor
                propertyName: "departureTime"
                initialValue: reservation.reservationFor.departureDay
                status: Kirigami.MessageType.Error
                statusMessage: departureTime.hasValue ? '' : i18nc("flight departure", "Departure time has to be set.")
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormTextFieldDelegate {
                id: departureGate
                text: reservation.reservationFor.departureGate
                label: i18nc("flight departure gate", "Gate")
            }
            FormCard.FormDelegateSeparator {}
            FormDateTimeEditDelegate {
                id: boardingTime
                text: i18n("Boarding time")
                obj: reservation.reservationFor
                propertyName: "boardingTime"
                initialValue: {
                    let d = new Date(departureTime.value);
                    d.setTime(d.getTime() - 30 * 60 * 1000);
                    return d;
                }
                status: Kirigami.MessageType.Warning
                statusMessage: {
                    if (boardingTime.hasValue && boardingTime.value > departureTime.value)
                        return i18nc("flight departure", "Boarding time has to be before the departure time.")
                    return '';
                }
            }
        }

        FormCard.FormHeader {
            title: i18nc("flight arrival", "Arrival - %1", reservation.reservationFor.arrivalAirport.iataCode)
        }

        FormCard.FormCard {
            FormCard.FormTextFieldDelegate {
                id: arrivalAirportName
                label: i18n("Airport")
                text: reservation.reservationFor.arrivalAirport.name
                status: Kirigami.MessageType.Error
                statusMessage: reservation.reservationFor.arrivalAirport.iataCode === "" && arrivalAirportName.text === "" ? i18n("Arrival airport has to be specified.") : ""
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormTextFieldDelegate {
                id: arrivalTerminal
                text: reservation.reservationFor.arrivalTerminal
                label: i18nc("flight arrival terminal", "Terminal")
            }
            FormCard.FormDelegateSeparator {}
            FormDateTimeEditDelegate {
                id: arrivalTime
                text: i18nc("flight arrival time", "Time")
                obj: reservation.reservationFor
                propertyName: "arrivalTime"
                initialValue: {
                    let d = new Date(departureTime.value);
                    d.setTime(d.getTime() + 120 * 60 * 1000);
                    return d;
                }
                status: Kirigami.MessageType.Error
                statusMessage: {
                    if (arrivalTime.hasValue && arrivalTime.value < departureTime.value)
                        return i18nc("flight arrival", "Arrival time has to be after the departure time.")
                    return '';
                }
            }
        }

        FormCard.FormHeader {
            title: i18n("Flight")
            visible: flightCard.visible
        }

        FormCard.FormCard {
            id: flightCard
            visible: root.reservation.reservedTicket.ticketToken === ""
            FormCard.FormTextFieldDelegate {
                id: flightNumber
                label: i18n("Flight number")
                text: root.reservation.reservationFor.airline.iataCode + " " + root.reservation.reservationFor.flightNumber
                status: Kirigami.MessageType.Warning
                statusMessage: flightNumber.text.match(/^\s*[A-Z0-9]{2}\s*\d{1,4}\s*$/) ? "" : i18n("Invalid flight number.")
            }
        }

        FormCard.FormHeader {
            title: i18nc("Flight seat", "Seat")
        }

        // TODO the below is per reservation, not per batch, so add a selector for that!
        FormCard.FormCard {
            FormCard.FormTextFieldDelegate {
                id: seat
                label: i18nc("Flight seat", "Seat")
                text: reservation.airplaneSeat
            }
        }

        BookingEditorCard {
            id: bookingEdit
            item: root.reservation
        }
    }
}
