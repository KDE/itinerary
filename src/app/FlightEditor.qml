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
    title: i18n("Edit Flight")

    function save(resId, reservation) {
        var flight = reservation.reservationFor;
        flight.departureTerminal = departureTerminal.text
        flight.departureGate = departureGate.text
        var newRes = reservation;
        newRes.reservationFor = flight;
        _reservationManager.updateReservation(resId, newRes);
    }

    GridLayout {
        id: grid
        width: parent.width
        columns: 2

        QQC2.Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: reservation.reservationFor.airline.iataCode + " " + reservation.reservationFor.flightNumber
            horizontalAlignment: Qt.AlignHCenter
            font.bold: true
        }

        // departure data
        QQC2.Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: i18n("Departure")
            horizontalAlignment: Qt.AlignHCenter
        }
        QQC2.Label {
            text: i18n("Departure terminal:")
        }
        QQC2.TextField {
            id: departureTerminal
            text: reservation.reservationFor.departureTerminal
        }
        QQC2.Label {
            text: i18n("Departure gate:")
        }
        QQC2.TextField {
            id: departureGate
            text: reservation.reservationFor.departureGate
        }
    }
}
