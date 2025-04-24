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

    /** Double tap on the barcode to request scan mode. */
    signal scanModeToggled()

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
        title: i18n("Vaccination")
    }

    FormCard.FormCard {
        FormCard.FormTextDelegate {
            readonly property int days: daysTo(certificate.date, new Date())
            text: i18n("Date")
            description: {
                const formattedDate = root.certificate.date.toLocaleDateString(Qt.locale(), Locale.ShortFormat);
                if (days > 0) {
                    return i18np("%2 (%1 day ago)", "%2 (%1 days ago)", days, formattedDate);
                }
                return formattedDate;
            }
            descriptionItem.color: root.certificate.vaccinationState != KHC.VaccinationCertificate.VaccinationTooRecent ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.neutralTextColor
            font.bold: true
            visible: !isNaN(root.certificate.date.getTime())
        }

        FormCard.FormDelegateSeparator { visible: root.certificate.disease }

        FormCard.FormTextDelegate {
            description: root.certificate.disease
            text: i18n("Disease")
            visible: root.certificate.disease
        }

        FormCard.FormDelegateSeparator { visible: root.certificate.vaccineType }

        FormCard.FormTextDelegate {
            description: root.certificate.vaccineType
            text: i18n("Type")
            visible: root.certificate.vaccineType
        }

        FormCard.FormDelegateSeparator { visible: root.certificate.vaccineUrl || root.certificate.vaccine }

        FormCard.FormTextDelegate {
            text: i18n("Vaccine")
            description: root.certificate.vaccineUrl != "" ? '<a href="' + root.certificate.vaccineUrl + '">' + root.certificate.vaccine + '</a>' : root.certificate.vaccine
            onLinkActivated: Qt.openUrlExternally(link)
            visible: description.length > 0
        }

        FormCard.FormDelegateSeparator { visible: root.certificate.manufacturer }

        FormCard.FormTextDelegate {
            text: i18n("Manufacturer")
            description: root.certificate.manufacturer
            visible: description.length > 0
        }

        FormCard.FormDelegateSeparator { visible: root.certificate.dose > 0 }

        FormCard.FormTextDelegate {
            text: i18n("Dose")
            description: root.certificate.totalDoses > 0 ? i18n("%1/%2", root.certificate.dose, root.certificate.totalDoses) : root.certificate.dose
            descriptionItem {
                color: root.certificate.dose < root.certificate.totalDoses ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.positiveTextColor
                font.bold: true
            }
            visible: root.certificate.dose > 0
        }

        FormCard.FormDelegateSeparator { visible: root.certificate.country }

        FormCard.FormTextDelegate {
            readonly property var country: Country.fromAlpha2(certificate.country)
            description: country.emojiFlag + " " + country.name
            text: i18n("Country")
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
            visible: description.length > 0
        }

        FormCard.FormDelegateSeparator { visible: root.certificate.certificateIssuer && root.certificate.certificateId }

        FormCard.FormTextDelegate {
            text: i18n("Identifier")
            description: root.certificate.certificateId
            visible: root.certificate.certificateId
        }

        FormCard.FormDelegateSeparator { visible: !isNaN(certificate.certificateIssueDate.getTime()) }

        FormCard.FormTextDelegate {
            text: i18n("Issued")
            description: root.certificate.certificateIssueDate.toLocaleString(Qt.locale(), Locale.ShortFormat)
            visible: !isNaN(certificate.certificateIssueDate.getTime())
        }

        FormCard.FormDelegateSeparator { visible: !isNaN(certificate.certificateExpiryDate.getTime()) }

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
