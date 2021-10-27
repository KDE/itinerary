/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import QtQuick.Window 2.10
import org.kde.kirigami 2.17 as Kirigami
import org.kde.prison 1.0 as Prison
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

ColumnLayout {
    id: root
    property var resIds

    readonly property var currentReservationId: ticketModel.reservationIdAt(travelerBox.currentIndex)
    readonly property var currentTicket: ticketModel.reservationAt(travelerBox.currentIndex) ? ticketModel.reservationAt(travelerBox.currentIndex).reservedTicket : undefined
    Layout.fillWidth: true

    /** There is a barcode displayed. */
    readonly property alias hasBarcode: background.visible
    /** Double tap on the barcode to request scan mode. */
    signal scanModeToggled()

    TicketTokenModel {
        id: ticketModel
        reservationManager: ReservationManager
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
            property bool is1dCode: barcode.dimensions == Prison.Barcode.OneDimension
            property bool isNonSquare: is1dCode || barcode.barcodeType == 7 // Prison.Barcode.PDF417 TODO replace once we depend on KF 5.88
            // if this gets too wide, we need to rotate by 90°
            property bool showVertical: isNonSquare && barcodeTargetWidth > root.width

            // unrotated barcode sizes
            property int barcodeTargetWidth: Math.max(barcode.implicitWidth + 2 * barcode.anchors.margins, Screen.pixelDensity * 50)
            property int barcodeTargetHeight: {
                if (is1dCode) {
                    return Screen.pixelDensity * 25;
                }
                if (isNonSquare) {
                    return barcode.implicitHeight + 2 * barcode.anchors.margins;
                }
                return barcodeTargetWidth;
            }

            implicitWidth: (showVertical ? barcodeTargetHeight : barcodeTargetWidth) + 2 * Kirigami.Units.smallSpacing
            implicitHeight: visible ? (showVertical ? barcodeTargetWidth : barcodeTargetHeight) + 2 * Kirigami.Units.smallSpacing : 0
            visible: barcode.implicitHeight > 0

            MouseArea {
                anchors.fill: parent
                onDoubleClicked: root.scanModeToggled()
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
                        case Ticket.PDF417: return 7; // Prison.Barcode.PDF417; TODO replace once we depend on KF 5.88
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
