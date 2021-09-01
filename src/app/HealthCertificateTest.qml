/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.prison 1.0 as Prison
import org.kde.khealthcertificate 1.0 as KHC
import org.kde.itinerary 1.0
import "." as App

ColumnLayout {
    id: root
    width: parent.width
    property var certificate

    /** Double tap on the barcode to request scan mode. */
    signal scanModeToggled()

    function daysTo(d1, d2) {
        return (d2.getTime() - d1.getTime()) / (1000 * 60 * 60 * 24);
    }

    App.HealthCertificateBarcode {
        certificate: root.certificate
        TapHandler {
            onDoubleTapped: root.scanModeToggled()
        }
    }

    Kirigami.FormLayout {
        Layout.fillWidth: true

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Person")
        }

        QQC2.Label {
            text: certificate.name
            Kirigami.FormData.label: i18n("Name:")
        }
        QQC2.Label {
            text: certificate.dateOfBirth.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
            visible: !isNaN(certificate.dateOfBirth.getTime())
            Kirigami.FormData.label: i18n("Date of birth:")
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Test")
        }

        QQC2.Label {
            text: certificate.date.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
            Kirigami.FormData.label: i18n("Date:")
            color: daysTo(certificate.date, new Date()) >= 2 ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.textColor
        }
        QQC2.Label {
            text: certificate.disease
            Kirigami.FormData.label: i18n("Disease:")
            visible: certificate.disease
        }
        QQC2.Label {
            text: certificate.testType
            Kirigami.FormData.label: i18n("Type:")
            visible: certificate.testType
        }
        QQC2.Label {
            text: certificate.testUrl != "" ? '<a href="' + certificate.testUrl + '">' + certificate.testName + '</a>' : certificate.testName
            visible: certificate.testName.length > 0
            Kirigami.FormData.label: i18n("Test:")
            onLinkActivated: Qt.openUrlExternally(link)
        }
        QQC2.Label {
            text: certificate.resultString
            Kirigami.FormData.label: i18n("Result:")
            color: certificate.result == KHC.TestCertificate.Positive ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
        }
        QQC2.Label {
            text: certificate.testCenter
            Kirigami.FormData.label: i18n("Test Center:")
        }
        QQC2.Label {
            text: Localizer.countryFlag(certificate.country) + " " + Localizer.countryName(certificate.country)
            Kirigami.FormData.label: i18n("Country:")
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Certificate")
        }

        QQC2.Label {
            text: certificate.certificateIssuer
            Kirigami.FormData.label: i18n("Issuer:")
        }
        QQC2.Label {
            text: certificate.certificateId
            Kirigami.FormData.label: i18n("Identifier:")
            wrapMode: Text.Wrap
        }
        QQC2.Label {
            text: certificate.certificateIssueDate.toLocaleString(Qt.locale(), Locale.ShortFormat)
            Kirigami.FormData.label: i18n("Issued:")
        }
        QQC2.Label {
            text: certificate.certificateExpiryDate.toLocaleString(Qt.locale(), Locale.ShortFormat)
            Kirigami.FormData.label: i18n("Expires:")
            visible: !isNaN(certificate.certificateExpiryDate.getTime())
        }
        Kirigami.Icon {
            source: {
                switch(certificate.signatureState) {
                    case KHC.HealthCertificate.ValidSignature: return "dialog-ok";
                    case KHC.HealthCertificate.UnknownSignature: return "question";
                    case KHC.HealthCertificate.InvalidSignature:
                    default:
                        return "dialog-error-symbolic";
                }
            }
            height: Kirigami.Units.gridUnit
            Kirigami.FormData.label: i18n("Signature:")
            color: {
                switch(certificate.signatureState) {
                    case KHC.HealthCertificate.ValidSignature: return Kirigami.Theme.positiveTextColor;
                    case KHC.HealthCertificate.UnknownSignature: return Kirigami.Theme.neutralTextColor;
                    case KHC.HealthCertificate.InvalidSignature:
                    default:
                        return Kirigami.Theme.negativeTextColor;
                }
            }
            visible: certificate.signatureState != KHC.HealthCertificate.UncheckedSignature
        }
    }
}
