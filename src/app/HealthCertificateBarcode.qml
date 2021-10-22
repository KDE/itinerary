/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.prison 1.0 as Prison
import org.kde.khealthcertificate 1.0 as KHC
import org.kde.itinerary 1.0

QQC2.Page {
    id: page
    required property var certificate

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    header: ColumnLayout {
        Kirigami.Heading {
            Layout.margins: Kirigami.Units.largeSpacing
            Layout.bottomMargin: 0
            Layout.fillWidth: true
            level: 2
            wrapMode: Text.WordWrap
            text: {
                const name = certificate.name.toLowerCase().replace(/(^|\s)\S/g, (t) => { return t.toUpperCase() });
                switch (certificate.type) {
                case KHC.HealthCertificate.Vaccination:
                    return i18n('Vaccination certificate of %1', name);
                case KHC.HealthCertificate.Test:
                    return i18n('Test certificate of %1', name);
                case KHC.HealthCertificate.Recovery:
                    return i18n('Recovery certificate of %1', name);
                default:
                    return '';
                }
            }
        }
        Kirigami.Heading {
            Layout.margins: Kirigami.Units.largeSpacing
            Layout.bottomMargin: 0
            Layout.topMargin: 0
            Layout.fillWidth: true
            level: 3
            wrapMode: Text.WordWrap
            text: {
                if (certificate.validationState) {
                    return i18n('Certificate invalid');
                }

                switch (certificate.type) {
                case KHC.HealthCertificate.Vaccination:
                switch (certificate.validationState) {
                    case KHC.HealthCertificate.Valid: return i18n('Fully vaccinated');
                    case KHC.HealthCertificate.Partial: return i18n('Partial vaccinated');
                }
                case KHC.HealthCertificate.Test:
                    case KHC.HealthCertificate.Valid: return i18n('Test valid');
                case KHC.HealthCertificate.Recovery:
                    return i18n('Recovery certificate valid');
                default:
                    return '';
                }
            }
        }
    }
    contentItem: Prison.Barcode {
        id: barcode
        barcodeType: Prison.Barcode.QRCode
        content: certificate.rawData
        anchors.centerIn: parent
    }

    footer: Item {
        height: Kirigami.Units.gridUnit * 4
    }

    background: Rectangle {
        opacity: 0.4
        color: switch (certificate.validationState) {
            case KHC.HealthCertificate.Valid: return Kirigami.Theme.positiveTextColor;
            case KHC.HealthCertificate.Partial: return Kirigami.Theme.neutralTextColor;
            case KHC.HealthCertificate.Invalid: return Kirigami.Theme.negativeTextColor;
            default: return "transparent"
        }
    }
}
