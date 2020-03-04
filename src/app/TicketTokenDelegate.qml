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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import QtQuick.Window 2.10
import org.kde.kirigami 2.0 as Kirigami
import org.kde.prison 1.0 as Prison
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

ColumnLayout {
    id: root
    property var resIds

    readonly property var currentReservationId: ticketModel.reservationIdAt(travelerBox.currentIndex)
    readonly property var currentTicket: ticketModel.reservationAt(travelerBox.currentIndex).reservedTicket
    Layout.fillWidth: true

    TicketTokenModel {
        id: ticketModel
        reservationManager: _reservationManager
        reservationIds: resIds
    }

    QQC2.ComboBox {
        id: travelerBox
        Layout.alignment: Qt.AlignCenter
        model: ticketModel
        textRole: "display"
        Layout.fillWidth: true

        // ugly, but rowCount does not trigger binding changes
        Component.onCompleted: visible = ticketModel.rowCount() >= 1 && root.resIds.length > 1
    }

    Item {
        id: barcodeContainer
        Layout.alignment: Qt.AlignCenter
        Layout.fillWidth: true
        implicitHeight: childrenRect.height

        Rectangle {
            id: background
            anchors.centerIn: barcodeContainer
            anchors.top: barcodeContainer.top
            anchors.bottom: barcodeContainer.bottom
            color: "white"
            // aim at 50x50mm for 2d codes, and 25mm height for 1d codes, if we have the space for it
            property bool is1dCode: currentTicket.ticketTokenType == Ticket.Code128
            // if this gets too wide, we need to rotate by 90°
            property bool showVertical: is1dCode && barcodeTargetWidth > root.width

            // unrotated barcode sizes
            property int barcodeTargetWidth: Math.max(
                (is1dCode ? 2 * barcode.implicitWidth : barcode.implicitWidth) + 2 * barcode.anchors.margins,
                Screen.pixelDensity * 50)
            property int barcodeTargetHeight: is1dCode ? Screen.pixelDensity * 25 : barcodeTargetWidth

            implicitWidth: (showVertical ? barcodeTargetHeight : barcodeTargetWidth) + 2 * Kirigami.Units.smallSpacing
            implicitHeight: visible ? (showVertical ? barcodeTargetWidth : barcodeTargetHeight) + 2 * Kirigami.Units.smallSpacing : 0
            visible: barcode.implicitHeight > 0

            MouseArea {
                anchors.fill: parent
                onDoubleClicked: {
                    _brightnessManager.toggleBrightness()
                    _lockManager.toggleInhibitScreenLock(i18n("In barcode scanning mode"))
                }
            }

            Prison.Barcode {
                id: barcode
                anchors.centerIn: background
                width: background.barcodeTargetWidth
                height: background.barcodeTargetHeight
                rotation: background.showVertical ? 90 : 0
                barcodeType:
                {
                    if (currentTicket == undefined)
                        return Prison.Barcode.Null;
                    switch (currentTicket.ticketTokenType) {
                        case Ticket.QRCode: return Prison.Barcode.QRCode;
                        case Ticket.AztecCode: return Prison.Barcode.Aztec;
                        case Ticket.Code128: return Prison.Barcode.Code128;
                        case Ticket.DataMatrix: return Prison.Barcode.DataMatrix;
                    }
                    return Prison.Barcode.Null;
                }
                content:
                {
                    if (barcodeType == Prison.Barcode.Null || currentTicket == undefined)
                        return "";
                    return currentTicket.ticketTokenData;
                }
            }
        }
    }
}
