/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.prison as Prison
import org.kde.khealthcertificate as KHC
import org.kde.itinerary

ColumnLayout {
    id: root
    width: parent.width
    required property var certificate

    function daysTo(d1, d2) {
        return (d2.getTime() - d1.getTime()) / (1000 * 60 * 60 * 24);
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
            text: Localizer.formatDate(certificate, "dateOfBirth")
            visible: !isNaN(certificate.dateOfBirth.getTime())
            Kirigami.FormData.label: i18n("Date of birth:")
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Recovery")
        }

        QQC2.Label {
            text: certificate.dateOfPositiveTest.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
            Kirigami.FormData.label: i18n("Positive test:")
        }
        QQC2.Label {
            text: certificate.disease
            Kirigami.FormData.label: i18n("Disease:")
        }
        QQC2.Label {
            text: certificate.validFrom.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
            Kirigami.FormData.label: i18n("Valid from:")
        }
        QQC2.Label {
            text: certificate.validUntil.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
            Kirigami.FormData.label: i18n("Valid until:")
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Certificate")
        }

        QQC2.Label {
            text: certificate.certificateIssuer
            Kirigami.FormData.label: i18n("Issuer:")
            visible: text !== ""
        }
        QQC2.Label {
            text: certificate.certificateId
            Kirigami.FormData.label: i18n("Identifier:")
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: certificate.certificateIssueDate.toLocaleString(Qt.locale(), Locale.ShortFormat)
            visible: !isNaN(certificate.certificateIssueDate.getTime())
            Kirigami.FormData.label: i18n("Issued:")
        }
        QQC2.Label {
            text: certificate.certificateExpiryDate.toLocaleString(Qt.locale(), Locale.ShortFormat)
            Kirigami.FormData.label: i18n("Expires:")
            visible: !isNaN(certificate.certificateExpiryDate.getTime())
            color: certificate.certificateExpiryDate.getTime() < Date.now() ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
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
