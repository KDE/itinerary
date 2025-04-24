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
import org.kde.i18n.localeData
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
        title: i18n("Test")
    }

    FormCard.FormCard {
        FormCard.FormTextDelegate {
            text: i18n("Date")
            description: root.certificate.date.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
            descriptionItem.color: daysTo(certificate.date, new Date()) >= 2 ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.textColor
            visible: !isNaN(certificate.date.getTime())
        }

        FormCard.FormDelegateSeparator { visible: root.certificate.disease && !isNaN(certificate.date.getTime()) }

        FormCard.FormTextDelegate {
            text: i18n("Disease")
            description: root.certificate.disease
            visible: root.certificate.disease
        }

        FormCard.FormDelegateSeparator { visible: root.certificate.testType }

        FormCard.FormTextDelegate {
            text: i18n("Type")
            description: root.certificate.testType
            visible: root.certificate.testType
        }

        FormCard.FormTextDelegate {
            text: i18n("Test")
            description: root.certificate.testUrl != "" ? '<a href="' + root.certificate.testUrl + '">' + root.certificate.testName + '</a>' : root.certificate.testName
            visible: root.certificate.testName.length > 0
            onLinkActivated: Qt.openUrlExternally(link)
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextDelegate {
            text: i18n("Result")
            description: {
                if (certificate.resultString !== "") {
                    return root.certificate.resultString;
                }
                switch (certificate.result) {
                    case KHC.TestCertificate.Positive:
                        return i18nc('test result', 'Positive');
                    case KHC.TestCertificate.Negative:
                        return i18nc('test result', 'Negative');
                }
            }
            descriptionItem.color: root.certificate.result == KHC.TestCertificate.Positive ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
        }

        FormCard.FormDelegateSeparator { visible: root.certificate.testCenter }

        FormCard.FormTextDelegate {
            text: i18n("Test Center")
            description: root.certificate.testCenter
            visible: description !== ""
        }

        FormCard.FormDelegateSeparator { visible: root.certificate.country }

        FormCard.FormTextDelegate {
            readonly property var country: Country.fromAlpha2(root.certificate.country)
            text: i18n("Country")
            description: country.emojiFlag + " " + country.name
            visible: root.certificate.country
        }
    }

    FormCard.FormHeader {
        title: i18n("Certificate")
    }

    FormCard.FormCard {
        FormCard.FormTextDelegate {
            text: i18n("Issuer")
            description: root.certificate.certificateIssuer
            visible: root.certificate.certificateIssuer !== ""
        }

        FormCard.FormDelegateSeparator { visible: root.certificate.certificateIssuer && root.certificate.certificateId }

        FormCard.FormTextDelegate {
            text: i18n("Identifier")
            description: root.certificate.certificateId
            visible: root.certificate.certificateId !== ""
        }

        FormCard.FormDelegateSeparator { visible: !isNaN(root.certificate.certificateIssueDate.getTime()) }

        FormCard.FormTextDelegate {
            text: i18n("Issued")
            description: root.certificate.certificateIssueDate.toLocaleString(Qt.locale(), Locale.ShortFormat)
            visible: !isNaN(root.certificate.certificateIssueDate.getTime())
        }

        FormCard.FormDelegateSeparator { visible: !isNaN(root.certificate.certificateExpiryDate.getTime()) }

        FormCard.FormTextDelegate {
            text: i18n("Expires")
            description: root.certificate.certificateExpiryDate.toLocaleString(Qt.locale(), Locale.ShortFormat)
            descriptionItem.color: root.certificate.certificateExpiryDate.getTime() < Date.now() ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
            visible: !isNaN(root.certificate.certificateExpiryDate.getTime())
        }

        FormCard.FormDelegateSeparator { visible: root.certificate.signatureState != KHC.HealthCertificate.UncheckedSignature }

        FormCard.FormTextDelegate {
            icon.name: {
                switch(certificate.signatureState) {
                    case KHC.HealthCertificate.ValidSignature: return "dialog-ok";
                    case KHC.HealthCertificate.UnknownSignature: return "question";
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
                    case KHC.HealthCertificate.ValidSignature: return Kirigami.Theme.positiveTextColor;
                    case KHC.HealthCertificate.UnknownSignature: return Kirigami.Theme.neutralTextColor;
                    case KHC.HealthCertificate.InvalidSignature:
                    default:
                        return Kirigami.Theme.negativeTextColor;
                }
            }
            visible: root.certificate.signatureState != KHC.HealthCertificate.UncheckedSignature
        }
    }
}
