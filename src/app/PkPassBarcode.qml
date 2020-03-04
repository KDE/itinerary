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
import org.kde.pkpass 1.0 as KPkPass
import org.kde.prison 1.0 as Prison

Rectangle {
    property var pass

    implicitHeight: barcodeLayout.implicitHeight
    implicitWidth: barcodeLayout.implicitWidth
    color: "white"
    radius: 6
    Layout.alignment: Qt.AlignCenter

    MouseArea {
        anchors.fill: parent
        onDoubleClicked: {
            _brightnessManager.toggleBrightness()
            _lockManager.toggleInhibitScreenLock(i18n("In barcode scanning mode"))
        }
    }

    ColumnLayout {
        id: barcodeLayout
        anchors.fill: parent
        Prison.Barcode {
            Layout.alignment: Qt.AlignCenter
            Layout.margins: 4
            Layout.preferredWidth: Math.max(implicitWidth, 0.8 * bodyBackground.width)
            Layout.preferredHeight: Layout.preferredWidth
            barcodeType: pass.barcodes[0].format == KPkPass.Barcode.QR ? Prison.Barcode.QRCode : Prison.Barcode.Aztec
            content: pass.barcodes[0].message
        }

        QQC2.Label {
            Layout.alignment: Qt.AlignCenter
            text: pass.barcodes[0].alternativeText
            color: "black"
            visible: text.length > 0
        }
    }
}
