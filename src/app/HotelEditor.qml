/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

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
    title: i18n("Edit Hotel Reservation")

    function save(resId, reservation) {
        var hotel = address.save(reservation.reservationFor);
        if (hotelName.text) {
            hotel.name = hotelName.text;
        }
        hotel = contactEdit.save(hotel);
        var newRes = reservation;
        newRes.reservationFor = hotel;

        if (checkinEdit.isModified)
            newRes = Util.setDateTimePreserveTimezone(newRes, "checkinTime", checkinEdit.value);
        if (checkoutEdit.isModified)
            newRes = Util.setDateTimePreserveTimezone(newRes, "checkoutTime", checkoutEdit.value);

        ReservationManager.updateReservation(resId, newRes);
    }

    ColumnLayout {
        width: root.width

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Hotel")
                }
                MobileForm.FormDelegateSeparator {}
                MobileForm.FormTextFieldDelegate {
                    id: hotelName
                    label: i18nc("hotel name", "Name")
                    text: reservation.reservationFor.name
                }
                MobileForm.FormDelegateSeparator {}
                App.FormPlaceEditorDelegate {
                    id: address
                    place: reservation.reservationFor
                    defaultCountry: {
                        const HOUR = 60 * 60 * 1000;
                        const DAY = 24 * HOUR;
                        let dt = reservation.checkinTime;
                        dt.setTime(dt.getTime() - (dt.getHours() * HOUR) + DAY);
                        return countryAtTime(dt);
                    }
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                App.FormDateTimeEditDelegate {
                    id: checkinEdit
                    text: i18nc("hotel checkin", "Check-in")
                    obj: reservation
                    propertyName: "checkinTime"
                }
                MobileForm.FormDelegateSeparator {}
                App.FormDateTimeEditDelegate {
                    id: checkoutEdit
                    text: i18nc("hotel checkout", "Check-out")
                    obj: reservation
                    propertyName: "checkoutTime"
                }
            }
        }

        App.ContactEditorCard {
            id: contactEdit
            contact: reservation.reservationFor
        }
    }
}
