/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.EditorPage {
    id: root
    title: i18n("Edit Hotel Reservation")

    function save(resId, reservation) {
        var hotel = address.save(reservation.reservationFor);
        hotel.name = address.name;
        hotel.telephone = phoneNumber.text;
        hotel.email = emailAddress.text;
        var newRes = reservation;
        newRes.reservationFor = hotel;
        ReservationManager.updateReservation(resId, newRes);
    }

    ColumnLayout {
        width: root.width

        App.PlaceEditor {
            id: address
            twinFormLayouts: [address, bottomLayout]
            Kirigami.FormData.isSection: true
            place: reservation.reservationFor
            name: reservation.reservationFor.name
        }

        Kirigami.FormLayout {
            id: bottomLayout
            twinFormLayouts: [address, bottomLayout]
            QQC2.TextField {
                id: phoneNumber
                Kirigami.FormData.label: i18n("Telephone:")
                text: reservationFor.telephone
            }
            QQC2.TextField {
                id: emailAddress
                Kirigami.FormData.label: i18n("Email:")
                text: reservationFor.email
            }

            // TODO
            // checkin time
            // checkout time
        }
    }
}
