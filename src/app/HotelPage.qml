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

App.DetailsPage {
    id: root
    title: qsTr("Hotel Reservation")

    GridLayout {
        id: grid
        anchors.left: parent.left
        anchors.right: parent.right
        columns: 2

        QQC2.Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: reservation.reservationFor.name
            horizontalAlignment: Qt.AlignHCenter
            font.bold: true
        }

        App.PlaceDelegate {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            place: reservation.reservationFor
        }

        // TODO make these interactive
        QQC2.Label {
            text: qsTr("Telephone:")
        }
        QQC2.Label {
            text: reservation.reservationFor.telephone
        }
        QQC2.Label {
            text: qsTr("Email:")
        }
        QQC2.Label {
            text: reservation.reservationFor.email
        }

        QQC2.Label {
            text: qsTr("Check-in time:")
        }
        QQC2.Label {
            text: Localizer.formatDateTime(reservation, "checkinTime")
        }
        QQC2.Label {
            text: qsTr("Check-out time:")
        }
        QQC2.Label {
            text: Localizer.formatDateTime(reservation, "checkoutTime")
        }

        QQC2.Label {
            text: qsTr("Booking reference:")
        }
        QQC2.Label {
            text: reservation.reservationNumber
        }
        QQC2.Label {
            text: qsTr("Under name:")
        }
        QQC2.Label {
            text: reservation.underName.name
        }
    }
}
