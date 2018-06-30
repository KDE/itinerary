/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.0 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.EditorPage {
    id: root
    title: i18n("Edit Restaurant")

    function save(resId, reservation) {
        var foodEstablishment = address.save(reservation.reservationFor)
        var newRes = reservation;
        newRes.reservationFor = foodEstablishment;
        _reservationManager.updateReservation(resId, newRes);
    }

    GridLayout {
        id: grid
        width: parent.width
        columns: 2

        QQC2.Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: reservation.reservationFor.name
            horizontalAlignment: Qt.AlignHCenter
            font.bold: true
        }

        // location
        App.PlaceEditor {
            id: address
            place: reservation.reservationFor
        }

        // time
        // TODO
    }
}
