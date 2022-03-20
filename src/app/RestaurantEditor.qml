/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.EditorPage {
    id: root
    title: i18n("Edit Restaurant")

    function save(resId, reservation) {
        var foodEstablishment = address.save(reservation.reservationFor)
        foodEstablishment.name = address.name;
        var newRes = reservation;
        newRes.reservationFor = foodEstablishment;
        ReservationManager.updateReservation(resId, newRes);
    }

    ColumnLayout {
        width: parent.width

        // location
        App.PlaceEditor {
            id: address
            place: reservation.reservationFor
            name: reservation.reservationFor.name
            defaultCountry: countryAtTime(reservation.startTime)
        }

        // time
        // TODO
    }
}
