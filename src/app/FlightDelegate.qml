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
import org.kde.kirigami 2.4 as Kirigami
import org.kde.itinerary 1.0
import "." as App

App.TimelineDelegate {
    id: root

    header: Rectangle {
        id: headerBackground
        Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
        implicitHeight: headerLayout.implicitHeight + Kirigami.Units.largeSpacing * 2
        anchors.leftMargin: -root.leftPadding
        anchors.topMargin: -root.topPadding
        anchors.rightMargin: -root.rightPadding

        RowLayout {
            id: headerLayout
            anchors.fill: parent
            anchors.margins: Kirigami.Units.largeSpacing

            QQC2.Label {
                text: qsTr("✈ %1 %2 %3 → %4")
                    .arg(reservation.reservationFor.airline.iataCode)
                    .arg(reservation.reservationFor.flightNumber)
                    .arg(reservation.reservationFor.departureAirport.iataCode)
                    .arg(reservation.reservationFor.arrivalAirport.iataCode)
                color: Kirigami.Theme.textColor
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
                Layout.fillWidth: true
            }
            QQC2.Label {
                text: isNaN(reservation.reservationFor.boardingTime.getTime()) ?
                    Localizer.formatTime(reservation.reservationFor, "departureTime") :
                    Localizer.formatTime(reservation.reservationFor, "boardingTime")
                color: Kirigami.Theme.textColor
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.headerFontScale
            }
        }
    }

    contentItem: GridLayout {
        id: topLayout
        columns: 2

        QQC2.Label {
            text: qsTr("Departure from %1: %2")
                .arg(reservation.reservationFor.departureAirport.name)
                .arg(Localizer.formatTime(reservation.reservationFor, "departureTime"))
            color: Kirigami.Theme.textColor
            wrapMode: Text.WordWrap
            Layout.maximumWidth: root.width
        }
        App.PlaceDelegate {
            place: reservation.reservationFor.departureAirport
            Layout.fillWidth: true
        }
        Kirigami.Separator {
            Layout.columnSpan: 2
            Layout.fillWidth: true
        }

        QQC2.Label {
            text: qsTr("Arrival at %1: %2")
                .arg(reservation.reservationFor.arrivalAirport.name)
                .arg(Localizer.formatDateTime(reservation.reservationFor, "arrivalTime"))
            color: Kirigami.Theme.textColor
            wrapMode: Text.WordWrap
            Layout.maximumWidth: root.width
        }

        App.PlaceDelegate {
            place: reservation.reservationFor.arrivalAirport
            Layout.fillWidth: true
        }

        QQC2.Label {
            Layout.columnSpan: 2
            text: qsTr("Terminal: %1  Gate: %2  Seat: %3")
                .arg(reservation.reservationFor.departureTerminal)
                .arg(reservation.reservationFor.departureGate)
                .arg(reservation.airplaneSeat)
            color: Kirigami.Theme.textColor
        }

        QQC2.Button {
            Layout.columnSpan: 2
            Layout.alignment: Qt.AlignHCenter
            text: root.passId !== "" ? qsTr("Boarding Pass") : qsTr("🎫 Ticket")
            onClicked: {
                if (root.passId !== "")
                    showBoardingPass();
                else
                    showTicket();
            }
            visible: root.passId !== "" || (root.reservation.reservedTicket != undefined && root.reservation.reservedTicket.ticketToken != "")
            icon.source: root.passId !== "" ? "image://org.kde.pkpass/" + passId + "/icon" : ""
        }
    }
}
