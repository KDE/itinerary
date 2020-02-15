/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
        hotel.name = name.text;
        hotel.telephone = phoneNumber.text;
        hotel.email = emailAddress.text;
        var newRes = reservation;
        newRes.reservationFor = hotel;
        _reservationManager.updateReservation(resId, newRes);
    }

    Kirigami.FormLayout {
        width: root.width

        QQC2.TextField {
            id: name
            Kirigami.FormData.label: i18n("Name:")
            text: reservationFor.name
        }
        App.PlaceEditor {
            id: address
            Kirigami.FormData.isSection: true
            place: reservation.reservationFor
        }

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
