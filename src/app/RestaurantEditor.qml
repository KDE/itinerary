/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.EditorPage {
    id: root

    title: i18nc("@title", "Edit Restaurant")

    isValidInput: startTimeEdit.hasValue && restaurantName.text !== "" && (!endTimeEdit.hasValue || startTimeEdit.value < endTimeEdit.value)

    function apply(reservation) {
        var foodEstablishment = address.save(reservation.reservationFor)
        if (restaurantName.text) {
            foodEstablishment.name = restaurantName.text;
        }
        foodEstablishment = contactEdit.save(foodEstablishment);

        var newRes = reservation;
        newRes.reservationFor = foodEstablishment;
        if (startTimeEdit.isModified)
            newRes = Util.setDateTimePreserveTimezone(newRes, "startTime", startTimeEdit.value);
        if (endTimeEdit.isModified)
            newRes = Util.setDateTimePreserveTimezone(newRes, "endTime", endTimeEdit.value);

        bookingEdit.apply(newRes);
        return newRes;
    }

    ColumnLayout {
        spacing: 0

        App.CardPageTitle {
            emojiIcon: "🍽️"
            text: i18n("Restaurant")

            Layout.fillWidth: true
        }

        FormCard.FormCard {
            FormCard.FormTextFieldDelegate {
                id: restaurantName
                label: i18nc("restaurant name", "Name")
                text: reservation.reservationFor.name
                status: Kirigami.MessageType.Error
                statusMessage: restaurantName.text === "" ? i18n("Name must not be empty.") : ""
            }

            FormCard.FormDelegateSeparator {}

            App.FormPlaceEditorDelegate {
                id: address
                place: {
                    if (root.batchId || !root.reservation.reservationFor.address.isEmpty || root.reservation.reservationFor.geo.isValid)
                        return reservation.reservationFor
                    return cityAtTime(reservation.startTime);
                }
            }
        }

        FormCard.FormHeader {
            title: i18nc("@title:group", "Reservation")
        }

        FormCard.FormCard {
            App.FormDateTimeEditDelegate {
                id: startTimeEdit
                text: i18n("Start Time")
                obj: reservation
                propertyName: "startTime"
                status: Kirigami.MessageType.Error
                statusMessage: startTimeEdit.hasValue ? '' : i18n("Start time has to be set.")
            }

            FormCard.FormDelegateSeparator {}

            App.FormDateTimeEditDelegate {
                id: endTimeEdit
                text: i18n("End Time")
                obj: reservation
                propertyName: "endTime"
                initialValue: {
                    let d = new Date(startTimeEdit.value);
                    d.setHours(d.getHours() + 2);
                    return d;
                }
                status: Kirigami.MessageType.Error
                statusMessage: {
                    if (endTimeEdit.hasValue && endTimeEdit.value < startTimeEdit.value)
                        return i18n("End time has to be after the start time.")
                    return '';
                }
            }
        }

        App.ContactEditorCard {
            id: contactEdit
            contact: reservation.reservationFor
        }

        App.BookingEditorCard {
            id: bookingEdit
            item: reservation
        }
    }
}
