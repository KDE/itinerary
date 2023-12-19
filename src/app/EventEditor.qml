/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

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
        spacing: 0

        FormCard.FormHeader {
            title: i18nc("event as in concert/conference/show, not as in appointment", "Event")
        }

        FormCard.FormCard {
            FormCard.FormTextFieldDelegate {
                id: eventName
                label: i18nc("event name", "Name")
                text: reservation.reservationFor.name
                status: Kirigami.MessageType.Error
                statusMessage: text === "" ? i18n("Name must not be empty.") : ""
            }
            FormCard.FormDelegateSeparator {}
            FormDateTimeEditDelegate {
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
            FormCard.FormDelegateSeparator {}
            FormDateTimeEditDelegate {
                id: startDateEdit
                text: i18n("Start Time")
                obj: reservation.reservationFor
                propertyName: "startDate"
                status: Kirigami.MessageType.Error
                statusMessage: startDateEdit.hasValue ? '' : i18n("Start time has to be set.")
            }
            FormCard.FormDelegateSeparator {}
            FormDateTimeEditDelegate {
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
            FormCard.FormDelegateSeparator {}
            FormCard.FormTextFieldDelegate {
                id: urlEdit
                label: i18n("Website")
                text: root.reservation.reservationFor.url
                inputMethodHints: Qt.ImhUrlCharactersOnly
            }
        }

        FormCard.FormHeader {
            title: i18n("Venue")
        }

        FormCard.FormCard {
            FormCard.FormTextFieldDelegate {
                id: venueName
                label: i18nc("venue name", "Name")
                text: reservation.reservationFor.location ? reservation.reservationFor.location.name : ""
            }
            FormCard.FormDelegateSeparator {}
            FormPlaceEditorDelegate {
                id: address
                place: {
                    if (root.reservation.reservationFor.location)
                        return root.reservation.reservationFor.location;
                    return cityAtTime(root.reservation.reservationFor.startDate);
                }
            }
        }

        // TODO seat

        BookingEditorCard {
            id: bookingEdit
            item: reservation
            defaultCurrency: Country.fromAlpha2(address.currentCountry).currencyCode
        }
    }
}
