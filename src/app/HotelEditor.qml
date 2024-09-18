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

    title: root.isNew ? i18n("New Hotel Reservation") : i18n("Edit Hotel Reservation")

    isValidInput: checkinEdit.hasValue && checkoutEdit.hasValue && hotelName.text !== "" && checkinEdit.value < checkoutEdit.value
    tripGroupSelector: tripGroupSelector

    function apply(reservation) {
        let hotel = address.save(reservation.reservationFor);
        if (hotelName.text) {
            hotel.name = hotelName.text;
        }
        hotel = contactEdit.save(hotel);
        let newRes = reservation;
        newRes.reservationFor = hotel;

        if (checkinEdit.isModified)
            newRes = Util.setDateTimePreserveTimezone(newRes, "checkinTime", checkinEdit.value);
        if (checkoutEdit.isModified)
            newRes = Util.setDateTimePreserveTimezone(newRes, "checkoutTime", checkoutEdit.value);

        bookingEdit.apply(newRes);
        return newRes;
    }

    ColumnLayout {
        spacing: 0

        CardPageTitle {
            emojiIcon: "🏨"
            text: i18n("Hotel")
        }

        FormCard.FormHeader {
            title: i18n("Trip")
            visible: root.isNew
        }

        TripGroupSelectorCard {
            id: tripGroupSelector
            visible: root.isNew
            suggestedName: hotelName.text
            tripGroupCandidates: TripGroupModel.intersectingXorAdjacentTripGroups(checkinEdit.value, checkoutEdit.hasValue ? checkoutEdit.value : checkinEdit.value)
        }

        FormCard.FormHeader {
            title: i18nc("@title:group", "Accommodation")
        }

        FormCard.FormCard {
            FormCard.FormTextFieldDelegate {
                id: hotelName
                label: i18nc("hotel name", "Name")
                text: reservation.reservationFor.name
                status: Kirigami.MessageType.Error
                statusMessage: text === "" ? i18n("Name must not be empty.") : ""
            }

            FormCard.FormDelegateSeparator {}

            FormPlaceEditorDelegate {
                id: address
                place: {
                    if (root.batchId || !root.reservation.reservationFor.address.isEmpty || root.reservation.reservationFor.geo.isValid)
                        return reservation.reservationFor

                    const HOUR = 60 * 60 * 1000;
                    const DAY = 24 * HOUR;
                    let dt = reservation.checkinTime;
                    dt.setTime(dt.getTime() - (dt.getHours() * HOUR) + DAY);
                    return cityAtTime(dt);
                }
            }
        }

        FormCard.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing

            FormDateTimeEditDelegate {
                id: checkinEdit
                text: i18nc("hotel checkin", "Check-in")
                obj: reservation
                propertyName: "checkinTime"
                status: Kirigami.MessageType.Error
                statusMessage: checkinEdit.hasValue ? '' : i18n("Check-in time has to be set.")
            }
            FormCard.FormDelegateSeparator {}
            FormDateTimeEditDelegate {
                id: checkoutEdit
                text: i18nc("hotel checkout", "Check-out")
                obj: reservation
                propertyName: "checkoutTime"
                initialValue: {
                    let d = new Date(checkinEdit.value);
                    d.setDate(d.getDate() + 1);
                    d.setHours(12);
                    return d;
                }
                status: Kirigami.MessageType.Error
                statusMessage: {
                    if (!checkoutEdit.hasValue)
                        return i18n("Check-out time has to be set.")
                    if (checkinEdit.hasValue && checkoutEdit.value < checkinEdit.value)
                        return i18n("Check-out time has to be after the check-in time.")
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
            defaultCurrency: Country.fromAlpha2(address.currentCountry).currencyCode
        }
    }
}
