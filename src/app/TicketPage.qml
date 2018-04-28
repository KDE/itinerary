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
import org.kde.prison 1.0 as Prison
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.Page {
    id: root
    title: qsTr("Ticket")
    property variant reservation

    ColumnLayout {
        anchors.fill: parent

        Rectangle {
            implicitHeight: barcodeLayout.implicitHeight
            implicitWidth: barcodeLayout.implicitWidth
            color: "white"
            Layout.alignment: Qt.AlignCenter

            ColumnLayout {
                id: barcodeLayout
                anchors.fill: parent

                Prison.Barcode {
                    id: barcode
                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: root.width * 0.8
                    Layout.preferredHeight: Layout.preferredWidth
                    Layout.margins: 4
                    barcodeType:
                    {
                        console.log(reservation.reservedTicket.ticketTokenType, Ticket.AztecCode);
                        if (reservation == undefined || reservation.reservedTicket == undefined)
                            return Prison.Barcode.Null;
                        switch (reservation.reservedTicket.ticketTokenType) {
                            case Ticket.QRCode: return Prison.Barcode.QRCode;
                            case Ticket.AztecCode: return Prison.Barcode.Aztec;
                        }
                        return Prison.Barcode.Null;
                    }
                    content:
                    {
                        if (barcodeType == Prison.Barcode.Null || reservation == undefined || reservation.reservedTicket == undefined)
                            return "";
                        return reservation.reservedTicket.ticketTokenData;
                    }
                }
            }
        }
    }

    onBackRequested: pageStack.pop()
}
