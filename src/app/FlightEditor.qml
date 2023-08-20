/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.EditorPage {
    id: root
    title: i18n("Edit Flight")

    isValidInput: departureTime.hasValue && (!arrivalTime.hasValue || departureTime.value < arrivalTime.value)

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

        var newRes = reservation;
        newRes.airplaneSeat = seat.text;
        newRes.reservationFor = flight;
        bookingEdit.apply(newRes);
        return newRes;
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
                    text: reservation.reservationFor.airline.iataCode + " " + reservation.reservationFor.flightNumber
                    horizontalAlignment: Qt.AlignHCenter
                    font.bold: true
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18nc("flight departure", "Departure - %1", reservation.reservationFor.departureAirport.iataCode)
                }
                MobileForm.FormTextFieldDelegate {
                    id: departureAirportName
                    label: i18n("Airport")
                    text: reservation.reservationFor.departureAirport.name
                }
                MobileForm.FormDelegateSeparator {}
                MobileForm.FormTextFieldDelegate {
                    id: departureTerminal
                    text: reservation.reservationFor.departureTerminal
                    label: i18nc("flight departure terminal", "Terminal")
                }
                MobileForm.FormDelegateSeparator {}
                App.FormDateTimeEditDelegate {
                    id: departureTime
                    text: i18nc("flight departure time", "Time")
                    obj: reservation.reservationFor
                    propertyName: "departureTime"
                    initialValue: reservation.reservationFor.departureDay
                    status: Kirigami.MessageType.Error
                    statusMessage: departureTime.hasValue ? '' : i18nc("flight departure", "Departure time has to be set.")
                }
                MobileForm.FormDelegateSeparator {}
                MobileForm.FormTextFieldDelegate {
                    id: departureGate
                    text: reservation.reservationFor.departureGate
                    label: i18nc("flight departure gate", "Gate")
                }
                MobileForm.FormDelegateSeparator {}
                App.FormDateTimeEditDelegate {
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
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18nc("flight arrival", "Arrival - %1", reservation.reservationFor.arrivalAirport.iataCode)
                }
                MobileForm.FormTextFieldDelegate {
                    id: arrivalAirportName
                    label: i18n("Airport")
                    text: reservation.reservationFor.arrivalAirport.name
                }
                MobileForm.FormDelegateSeparator {}
                MobileForm.FormTextFieldDelegate {
                    id: arrivalTerminal
                    text: reservation.reservationFor.arrivalTerminal
                    label: i18nc("flight arrival terminal", "Terminal")
                }
                MobileForm.FormDelegateSeparator {}
                App.FormDateTimeEditDelegate {
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
        }

        // TODO the below is per reservation, not per batch, so add a selector for that!
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Seat")
                }
                MobileForm.FormTextFieldDelegate {
                    id: seat
                    label: i18n("Seat")
                    text: reservation.airplaneSeat
                }
            }
        }

        App.BookingEditorCard {
            id: bookingEdit
            item: root.reservation
        }
    }
}
