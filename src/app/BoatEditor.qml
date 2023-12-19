/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.i18n.localeData
import org.kde.kitinerary
import org.kde.itinerary

EditorPage {
    id: root
    title: i18n("Edit Boat Trip")

    isValidInput: departureTerminalName.text !== "" && arrivalTerminalName.text !== "" && departureTimeEdit.hasValue
        && (!arrivalTimeEdit.hasValue || departureTimeEdit.value < arrivalTimeEdit.value)

    function apply(reservation) {
        let trip = reservation.reservationFor;
        if (departureTimeEdit.isModified)
            trip = Util.setDateTimePreserveTimezone(trip, "departureTime", departureTimeEdit.value);
        if (arrivalTimeEdit.isModified)
            trip = Util.setDateTimePreserveTimezone(trip, "arrivalTime", departureTimeEdit.value);

        let dep = trip.departureBoatTerminal;
        dep.name = departureTerminalName.text;
        departureAddress.save(dep);
        trip.departureBoatTerminal = dep;

        let arr = trip.arrivalBoatTerminal;
        arr.name = arrivalTerminalName.text;
        arrivalAddress.save(arr);
        trip.arrivalBoatTerminal = arr;

        let newRes = reservation;
        newRes.reservationFor = trip;
        bookingEdit.apply(newRes);
        return newRes;
    }

    ColumnLayout {
        spacing: 0

        CardPageTitle {
            emojiIcon: "🛳️"
            text: i18n("Boat")
        }

        FormCard.FormHeader {
            title: i18nc("Boat departure", "Departure")
        }

        FormCard.FormCard {
            FormDateTimeEditDelegate {
                id: departureTimeEdit
                text: i18nc("Boat departure", "Departure Time")
                obj: reservation.reservationFor
                propertyName: "departureTime"
                status: Kirigami.MessageType.Error
                statusMessage: departureTimeEdit.hasValue ? '' : i18n("Departure time has to be set.")
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormTextFieldDelegate {
                id: departureTerminalName
                label: i18nc("boat terminal", "Terminal Name")
                text: reservation.reservationFor.departureBoatTerminal.name
                status: Kirigami.MessageType.Error
                statusMessage: text === "" ? i18n("Departure terminal must not be empty.") : ""
            }
            FormCard.FormDelegateSeparator {}
            FormPlaceEditorDelegate {
                id: departureAddress
                place: {
                    if (root.batchId || !root.reservation.reservationFor.departureBoatTerminal.address.isEmpty || root.reservation.reservationFor.departureBoatTerminal.geo.isValid)
                        return reservation.reservationFor.departureBoatTerminal;
                    return cityAtTime(root.reservation.reservationFor.departureTime);
                }
            }
        }

        FormCard.FormHeader {
            title: i18nc("Boat arrival", "Arrival")
        }

        FormCard.FormCard {
            FormDateTimeEditDelegate {
                id: arrivalTimeEdit
                text: i18nc("Boat arrival", "Arrival Time")
                obj: reservation.reservationFor
                propertyName: "arrivalTime"
                initialValue: {
                    let d = new Date(departureTimeEdit.value);
                    d.setHours(d.getHours() + 8);
                    return d;
                }
                status: Kirigami.MessageType.Error
                statusMessage: {
                    if (arrivalTimeEdit.hasValue && arrivalTimeEdit.value < departureTimeEdit.value)
                        return i18n("Arrival time has to be after the departure time.")
                    return '';
                }
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormTextFieldDelegate {
                id: arrivalTerminalName
                label: i18nc("boat terminal", "Terminal Name")
                text: reservation.reservationFor.arrivalBoatTerminal.name
                status: Kirigami.MessageType.Error
                statusMessage: text === "" ? i18n("Arrival terminal must not be empty.") : ""
            }
            FormCard.FormDelegateSeparator {}
            FormPlaceEditorDelegate {
                id: arrivalAddress
                place: {
                    if (root.batchId || !root.reservation.reservationFor.arrivalBoatTerminal.address.isEmpty || root.reservation.reservationFor.arrivalBoatTerminal.geo.isValid)
                        return reservation.reservationFor.arrivalBoatTerminal;
                    let p = cityAtTime(root.reservation.reservationFor.departureTime);
                    let addr = p.address;
                    addr.addressLocality = undefined;
                    addr.addressRegion = undefined;
                    p.address = addr;
                    return p;
                }
            }
        }

        BookingEditorCard {
            id: bookingEdit
            item: root.reservation
            defaultCurrency: Country.fromAlpha2(departureAddress.currentCountry).currencyCode
        }
    }
}
