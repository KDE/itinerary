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
import org.kde.itinerary 1.0
import "." as App

App.TimelineDelegate {
    id: root
    implicitHeight: topLayout.implicitHeight

    ColumnLayout {
        id: topLayout
        width: root.width

        Rectangle {
            id: headerBackground
            Layout.fillWidth: true
            color: Kirigami.Theme.complementaryBackgroundColor
            implicitHeight: headerLayout.implicitHeight

            RowLayout {
                id: headerLayout
                anchors.left: parent.left
                anchors.right: parent.right

                QQC2.Label {
                    text: qsTr("🚄 %1 %2")
                        .arg(reservation.reservationFor.trainName)
                        .arg(reservation.reservationFor.trainNumber)
                    color: Kirigami.Theme.complementaryTextColor
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
                    Layout.fillWidth: true
                }
                QQC2.Label {
                    text: Localizer.formatTime(reservation.reservationFor, "departureTime")
                    color: Kirigami.Theme.complementaryTextColor
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
                }
            }
        }

        QQC2.Label {
            text: qsTr("Departure from %1 on platform %2")
                .arg(reservation.reservationFor.departureStation.name)
                .arg(reservation.reservationFor.departurePlatform)
            color: Kirigami.Theme.textColor
        }
        App.PlaceDelegate {
            place: reservation.reservationFor.departureStation
            Layout.fillWidth: true
        }
        // TODO reserved seat
        QQC2.Label {
            text: qsTr("Arrival at %1 on platform %2")
                .arg(reservation.reservationFor.arrivalStation.name)
                .arg(reservation.reservationFor.arrivalPlatform)
            color: Kirigami.Theme.textColor
        }
        QQC2.Label {
            text: qsTr("Arrival time: %1")
                .arg(Localizer.formatDateTime(reservation.reservationFor, "arrivalTime"))
            color: Kirigami.Theme.textColor
        }
        App.PlaceDelegate {
            place: reservation.reservationFor.arrivalStation
            Layout.fillWidth: true
        }

        QQC2.Button {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("🎫 Ticket")
            onClicked: root.showTicket()
            visible: root.reservation.reservedTicket != undefined && root.reservation.reservedTicket.ticketToken != ""
        }
    }
}

