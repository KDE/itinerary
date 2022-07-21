/*
    SPDX-FileCopyrightText: 2018-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Window 2.15
import org.kde.kirigami 2.19 as Kirigami
import org.kde.prison 1.0 as Prison
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Item {
    id: barcodeContainer
    implicitHeight: childrenRect.height
    visible: barcode.implicitHeight > 0

    /** Barcode format to use, using the KItinerary::Ticket enum. */
    property var barcodeType
    /** Barcode content, as string or byte array. */
    property var barcodeContent

    /** Emitted when the barcode is double clicked, for toggling scan mode. */
    signal doubleClicked()

    Rectangle {
        id: background
        anchors.centerIn: barcodeContainer
        anchors.top: barcodeContainer.top
        anchors.bottom: barcodeContainer.bottom
        color: "white"
        // aim at 50x50mm for 2d codes, and 25mm height for 1d codes, if we have the space for it
        readonly property bool is1dCode: barcode.dimensions == Prison.Barcode.OneDimension
        readonly property bool isNonSquare: is1dCode || barcode.barcodeType == Prison.Barcode.PDF417
        // if this gets too wide, we need to rotate by 90°
        readonly property bool showVertical: isNonSquare && barcodeTargetWidth > barcodeContainer.width

        // unrotated barcode sizes
        readonly property int barcodeTargetWidth: Math.max(barcode.implicitWidth + 2 * barcode.anchors.margins, Screen.pixelDensity * 50)
        readonly property int barcodeTargetHeight: {
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

        MouseArea {
            anchors.fill: parent
            onDoubleClicked: barcodeContainer.doubleClicked()
        }

        Prison.Barcode {
            id: barcode
            anchors.centerIn: background
            width: background.showVertical ? background.barcodeTargetWidth : Math.min(background.barcodeTargetWidth, barcodeContainer.width)
            height: background.barcodeTargetHeight
            rotation: background.showVertical ? 90 : 0
            barcodeType:
            {
                if (barcodeContainer.barcodeType == undefined)
                    return Prison.Barcode.Null;
                switch (barcodeContainer.barcodeType) {
                    case Ticket.QRCode: return Prison.Barcode.QRCode;
                    case Ticket.AztecCode: return Prison.Barcode.Aztec;
                    case Ticket.Code128: return Prison.Barcode.Code128;
                    case Ticket.DataMatrix: return Prison.Barcode.DataMatrix;
                    case Ticket.PDF417: return Prison.Barcode.PDF417;
                    case Ticket.Code39: return Prison.Barcode.Code39;
                }
                return Prison.Barcode.Null;
            }
            content:
            {
                if (barcode.barcodeType == Prison.Barcode.Null || barcodeContainer.barcodeType == undefined)
                    return "";
                return barcodeContainer.barcodeContent;
            }
        }
    }
}
