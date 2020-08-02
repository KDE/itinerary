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
    title: i18n("Edit Event")

    function save(resId, reservation) {
        var event = reservation.reservationFor;
        var loc = address.save(reservation.reservationFor.location);
        loc.name = address.name;
        event.location = loc;
        var newRes = reservation;
        newRes.reservationFor = event;
        ReservationManager.updateReservation(resId, newRes);
    }

    ColumnLayout {
        width: root.width

        QQC2.Label {
            Layout.fillWidth: true
            text: reservationFor.name
            horizontalAlignment: Qt.AlignHCenter
            font.bold: true
            wrapMode: Text.WordWrap
        }

        App.PlaceEditor {
            id: address
            Kirigami.FormData.isSection: true
            nameLabel: i18n("Venue:")
            place: reservation.reservationFor.location
            name: reservation.reservationFor.location.name
        }

        // TODO start/end/entrance times
    }
}
