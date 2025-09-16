/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.pkpass as KPkPass
import org.kde.prison as Prison

Rectangle {
    id: root
    property int maximumWidth
    property KPkPass.Pass pass
    readonly property KPkPass.barcode barcode: {
        for (const bc of root.pass.barcodes) {
            // prefer 2D codes that work better on mobile phones when available
            if (bc.format === KPkPass.Barcode.QR || bc.format === KPkPass.Barcode.Aztec)
                return bc;
        }
        return root.pass.barcodes[0];
    }

    implicitHeight: barcodeLayout.implicitHeight
    implicitWidth: barcodeLayout.implicitWidth
    color: "white"
    radius: 6
    Layout.alignment: Qt.AlignCenter
    visible: root.pass && root.pass.barcodes.length > 0 && root.barcode.format !== KPkPass.Barcode.Invalid

    ColumnLayout {
        id: barcodeLayout
        anchors.centerIn: parent
        Prison.Barcode {
            id: barcode
            Layout.alignment: Qt.AlignCenter
            Layout.margins: 4
            Layout.preferredWidth: 0.8 * root.maximumWidth
            Layout.preferredHeight: implicitHeight * (Layout.preferredWidth / implicitWidth)
            barcodeType: {
                switch(root.barcode.format) {
                    case KPkPass.Barcode.QR: return Prison.Barcode.QRCode
                    case KPkPass.Barcode.Aztec: return Prison.Barcode.Aztec
                    case KPkPass.Barcode.PDF417: return Prison.Barcode.PDF417;
                    case KPkPass.Barcode.Code128: return Prison.Barcode.Code128;
                }
            }
            content: root.barcode.message
        }

        QQC2.Label {
            Layout.fillWidth: true
            Layout.maximumWidth: root.maximumWidth
            text: root.barcode.alternativeText
            color: "black"
            visible: text.length > 0
            wrapMode: Text.Wrap
            horizontalAlignment: Qt.AlignHCenter
        }
    }
}
