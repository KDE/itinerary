/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.prison 1.0 as Prison
import org.kde.solidextras 1.0 as Solid
import org.kde.khealthcertificate 1.0 as KHC
import org.kde.itinerary 1.0

Rectangle {
    id: validationBg
    property var certificate
    Layout.fillWidth: true
    implicitHeight: barcode.height + Kirigami.Units.largeSpacing * 4

    Prison.Barcode {
        anchors.centerIn: parent
        id: barcode
        barcodeType: Prison.Barcode.QRCode
        content: certificate.rawData
        width: Math.max(Math.floor(validationBg.width / minimumWidth) * minimumWidth, minimumWidth)
        height: width
    }

    color: switch (certificate.validationState) {
        case KHC.HealthCertificate.Valid: return Kirigami.Theme.positiveTextColor;
        case KHC.HealthCertificate.Partial: return Kirigami.Theme.neutralTextColor;
        case KHC.HealthCertificate.Invalid: return Kirigami.Theme.negativeTextColor;
        default: return "transparent"
    }

    MouseArea {
        anchors.fill: parent
        onDoubleClicked: {
            Solid.BrightnessManager.toggleBrightness()
            Solid.LockManager.toggleInhibitScreenLock(i18n("In barcode scanning mode"))
        }
    }
}
