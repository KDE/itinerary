/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.i18n.localeData 1.0
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.EditorPage {
    id: root
    title: i18nc("event as in concert/conference/show, not as in appointment", "Edit Event")

    isValidInput: eventName.text !== "" && startDateEdit.hasValue && (!endDateEdit.hasValue || startDateEdit.value < endDateEdit.value)

    function apply(reservation) {
        let event = reservation.reservationFor;
        event.name = eventName.text;
        let loc = address.save(reservation.reservationFor.location ? reservation.reservationFor.location : Factory.makePlace());
        loc.name = venueName.text;
        event.location = loc;
        event.url = urlEdit.text;

        if (entranceTimeEdit.isModified)
            event = Util.setDateTimePreserveTimezone(event, "doorTime", entranceTimeEdit.value);
        if (startDateEdit.isModified)
            event = Util.setDateTimePreserveTimezone(event, "startDate", startDateEdit.value);
        if (endDateEdit.isModified)
            event = Util.setDateTimePreserveTimezone(event, "endDate", endDateEdit.value);

        let newRes = reservation;
        newRes.reservationFor = event;
        bookingEdit.apply(newRes);
        return newRes;
    }

    ColumnLayout {
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18nc("event as in concert/conference/show, not as in appointment", "Event")
                }
                MobileForm.FormTextFieldDelegate {
                    id: eventName
                    label: i18nc("event name", "Name")
                    text: reservation.reservationFor.name
                    status: Kirigami.MessageType.Error
                    statusMessage: text === "" ? i18n("Name must not be empty.") : ""
                }
                MobileForm.FormDelegateSeparator {}
                App.FormDateTimeEditDelegate {
                    id: entranceTimeEdit
                    text: i18nc("time of entrance", "Entrance")
                    obj: reservation.reservationFor
                    propertyName: "doorTime"
                    initialValue: {
                        let d = new Date(startDateEdit.value);
                        d.setHours(d.getHours() - 2);
                        return d;
                    }
                    status: Kirigami.MessageType.Warning
                    statusMessage: {
                        if (entranceTimeEdit.hasValue && entranceTimeEdit.value > startDateEdit.value)
                            return i18n("Entrance time has to be before the start time.")
                        return '';
                    }
                }
                MobileForm.FormDelegateSeparator {}
                App.FormDateTimeEditDelegate {
                    id: startDateEdit
                    text: i18n("Start Time")
                    obj: reservation.reservationFor
                    propertyName: "startDate"
                    status: Kirigami.MessageType.Error
                    statusMessage: startDateEdit.hasValue ? '' : i18n("Start time has to be set.")
                }
                MobileForm.FormDelegateSeparator {}
                App.FormDateTimeEditDelegate {
                    id: endDateEdit
                    text: i18n("End Time")
                    obj: reservation.reservationFor
                    propertyName: "endDate"
                    initialValue: {
                        let d = new Date(startDateEdit.value);
                        d.setHours(d.getHours() + 2);
                        return d;
                    }
                    status: Kirigami.MessageType.Error
                    statusMessage: {
                        if (endDateEdit.hasValue && endDateEdit.value < startDateEdit.value)
                            return i18n("End time has to be after the start time.")
                        return '';
                    }
                }
                MobileForm.FormDelegateSeparator {}
                MobileForm.FormTextFieldDelegate {
                    id: urlEdit
                    label: i18n("Website")
                    text: root.reservation.reservationFor.url
                    inputMethodHints: Qt.ImhUrlCharactersOnly
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Venue")
                }
                MobileForm.FormTextFieldDelegate {
                    id: venueName
                    label: i18nc("venue name", "Name")
                    text: reservation.reservationFor.location ? reservation.reservationFor.location.name : ""
                }
                MobileForm.FormDelegateSeparator {}
                App.FormPlaceEditorDelegate {
                    id: address
                    place: {
                        if (root.reservation.reservationFor.location)
                            return root.reservation.reservationFor.location;
                        return cityAtTime(root.reservation.reservationFor.startDate);
                    }
                }
            }
        }

        // TODO seat

        App.BookingEditorCard {
            id: bookingEdit
            item: reservation
            defaultCurrency: Country.fromAlpha2(address.currentCountry).currencyCode
        }
    }
}
