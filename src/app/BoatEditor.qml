/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.EditorPage {
    id: root
    title: i18n("Edit Boat Trip")

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
        width: parent.width

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18nc("Boat departure", "Departure")
                }
                App.FormDateTimeEditDelegate {
                    id: departureTimeEdit
                    text: i18nc("Boat departure", "Departure Time")
                    obj: reservation.reservationFor
                    propertyName: "departureTime"
                }
                MobileForm.FormDelegateSeparator {}
                MobileForm.FormTextFieldDelegate {
                    id: departureTerminalName
                    label: i18nc("boat terminal", "Terminal Name")
                    text: reservation.reservationFor.departureBoatTerminal.name
                }
                MobileForm.FormDelegateSeparator {}
                App.FormPlaceEditorDelegate {
                    id: departureAddress
                    place: reservation.reservationFor.departureBoatTerminal
                    defaultCountry: countryAtTime(reservation.reservationFor.departureTime)
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18nc("Boat arrival", "Arrival")
                }
                App.FormDateTimeEditDelegate {
                    id: arrivalTimeEdit
                    text: i18nc("Boat arrival", "Arrival Time")
                    obj: reservation.reservationFor
                    propertyName: "arrivalTime"
                }
                MobileForm.FormDelegateSeparator {}
                MobileForm.FormTextFieldDelegate {
                    id: arrivalTerminalName
                    label: i18nc("boat terminal", "Terminal Name")
                    text: reservation.reservationFor.arrivalBoatTerminal.name
                }
                MobileForm.FormDelegateSeparator {}
                App.FormPlaceEditorDelegate {
                    id: arrivalAddress
                    place: reservation.reservationFor.arrivalBoatTerminal
                    defaultCountry: countryAtTime(reservation.reservationFor.arrivalTime)
                }
            }
        }

        App.BookingEditorCard {
            id: bookingEdit
            item: root.reservation
            defaultCurrency: Country.fromAlpha2(departureAddress.currentCountry).currencyCode
        }
    }
}
