/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.prison as Prison
import org.kde.khealthcertificate as KHC
import org.kde.itinerary

ColumnLayout {
    id: root

    required property var certificate

    function daysTo(d1: date, d2: date): int {
        return (d2.getTime() - d1.getTime()) / (1000 * 60 * 60 * 24);
    }

    width: parent.width
    spacing: 0

    FormCard.FormHeader {
        title: i18n("Person")
    }

    FormCard.FormCard {
        FormCard.FormTextDelegate {
            text: i18n("Name")
            description: root.certificate.name
        }

        FormCard.FormDelegateSeparator {
            visible: !isNaN(root.certificate.dateOfBirth.getTime())
        }

        FormCard.FormTextDelegate {
            text: i18n("Date of birth")
            description: Localizer.formatDate(root.certificate, "dateOfBirth")
            visible: !isNaN(root.certificate.dateOfBirth.getTime())
        }
    }

    FormCard.FormHeader {
        title: i18n("Recovery")
    }

    FormCard.FormCard {
        FormCard.FormTextDelegate {
            text: i18n("Positive test")
            description: root.certificate.dateOfPositiveTest.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextDelegate {
            text: i18n("Disease")
            description: root.certificate.disease
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextDelegate {
            text: i18n("Valid from")
            description: root.certificate.validFrom.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextDelegate {
            text: i18n("Valid until")
            description: root.certificate.validUntil.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
        }
    }

    FormCard.FormHeader {
        title: i18n("Certificate")
    }

    FormCard.FormCard {
        FormCard.FormTextDelegate {
            text: i18n("Issuer")
            description: root.certificate.certificateIssuer
            visible: description !== ""
        }

        FormCard.FormDelegateSeparator { visible: root.certificate.certificateIssuer.length > 0 }

        FormCard.FormTextDelegate {
            text: i18n("Identifier")
            description: root.certificate.certificateId
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextDelegate {
            text: root.certificate.certificateIssueDate.toLocaleString(Qt.locale(), Locale.ShortFormat)
            visible: !isNaN(certificate.certificateIssueDate.getTime())
            Kirigami.FormData.label: i18n("Issued")
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextDelegate {
            id: expiresDelegate
            text: i18n("Expires")
            description: root.certificate.certificateExpiryDate.toLocaleString(Qt.locale(), Locale.ShortFormat)
            visible: !isNaN(certificate.certificateExpiryDate.getTime())
            descriptionItem.color: root.certificate.certificateExpiryDate.getTime() < Date.now() ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextDelegate {
            icon.name: {
                switch(certificate.signatureState) {
                case KHC.HealthCertificate.ValidSignature:
                    return "dialog-ok-symbolic";
                case KHC.HealthCertificate.UnknownSignature:
                    return "question-symbolic";
                case KHC.HealthCertificate.InvalidSignature:
                default:
                    return "dialog-error-symbolic";
                }
            }
            text: i18n("Signature")
            description: {
                switch(certificate.signatureState) {
                case KHC.HealthCertificate.ValidSignature:
                    return i18nc("Signature state", "Valid");
                case KHC.HealthCertificate.UnknownSignature:
                    return i18nc("Signature state", "Unknown");
                case KHC.HealthCertificate.InvalidSignature:
                default:
                    return i18nc("Signature state", "Invalid");;
                }
            }
            descriptionItem.color: {
                switch(certificate.signatureState) {
                case KHC.HealthCertificate.ValidSignature:
                    return Kirigami.Theme.positiveTextColor;
                case KHC.HealthCertificate.UnknownSignature:
                    return Kirigami.Theme.neutralTextColor;
                case KHC.HealthCertificate.InvalidSignature:
                default:
                    return Kirigami.Theme.negativeTextColor;
                }
            }
            visible: root.certificate.signatureState != KHC.HealthCertificate.UncheckedSignature
        }
    }
}
