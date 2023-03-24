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
    title: i18n("Edit Restaurant")

    function save(resId, reservation) {
        var foodEstablishment = address.save(reservation.reservationFor)
        if (restaurantName.text) {
            foodEstablishment.name = restaurantName.text;
        }
        foodEstablishment = contactEdit.save(foodEstablishment);
        var newRes = reservation;
        newRes.reservationFor = foodEstablishment;
        ReservationManager.updateReservation(resId, newRes);
    }

    ColumnLayout {
        width: parent.width

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Restaurant")
                }
                MobileForm.FormDelegateSeparator {}
                MobileForm.FormTextFieldDelegate {
                    id: restaurantName
                    label: i18nc("restaurant name", "Name")
                    text: reservation.reservationFor.name
                }
                MobileForm.FormDelegateSeparator {}
                App.FormPlaceEditorDelegate {
                    id: address
                    place: reservation.reservationFor
                    defaultCountry: countryAtTime(reservation.startTime)
                }
            }
        }

        // time
        // TODO

        App.ContactEditorCard {
            id: contactEdit
            contact: reservation.reservationFor
        }
    }
}
