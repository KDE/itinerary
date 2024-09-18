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

    title: root.isNew ? i18nc("@title", "New Restaurant")
        : i18nc("@title", "Edit Restaurant")

    isValidInput: startTimeEdit.hasValue && restaurantName.text !== "" && (!endTimeEdit.hasValue || startTimeEdit.value < endTimeEdit.value)
    tripGroupSelector: tripGroupSelector

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

        CardPageTitle {
            emojiIcon: "🍽️"
            text: i18n("Restaurant")

            Layout.fillWidth: true
        }

        FormCard.FormHeader {
            title: i18n("Trip")
            visible: root.isNew
        }

        TripGroupSelectorCard {
            id: tripGroupSelector
            visible: root.isNew
            suggestedName: restaurantName.text
            tripGroupCandidates: TripGroupModel.intersectingXorAdjacentTripGroups(startTimeEdit.value, endTimeEdit.hasValue ? endTimeEdit.value : startTimeEdit.value)
        }

        FormCard.FormHeader {
            title: i18n("Restaurant")
            visible: root.isNew
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

            FormPlaceEditorDelegate {
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
            FormDateTimeEditDelegate {
                id: startTimeEdit
                text: i18n("Start Time")
                obj: reservation
                propertyName: "startTime"
                status: Kirigami.MessageType.Error
                statusMessage: startTimeEdit.hasValue ? '' : i18n("Start time has to be set.")
            }

            FormCard.FormDelegateSeparator {}

            FormDateTimeEditDelegate {
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

        ContactEditorCard {
            id: contactEdit
            contact: reservation.reservationFor
        }

        BookingEditorCard {
            id: bookingEdit
            item: reservation
        }
    }
}
