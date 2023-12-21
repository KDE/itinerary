/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.i18n.localeData
import org.kde.prison as Prison
import org.kde.khealthcertificate as KHC
import org.kde.itinerary

ColumnLayout {
    id: root
    width: parent.width
    property var certificate

    /** Double tap on the barcode to request scan mode. */
    signal scanModeToggled()

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
            Kirigami.FormData.label: i18n("Vaccination")
        }

        QQC2.Label {
            readonly property int days: daysTo(certificate.date, new Date())
            text: {
                const formattedDate = certificate.date.toLocaleDateString(Qt.locale(), Locale.ShortFormat);
                if (days > 0) {
                    return i18np("%2 (%1 day ago)", "%2 (%1 days ago)", days, formattedDate);
                }
                return formattedDate;
            }
            Kirigami.FormData.label: i18n("Date:")
            color: certificate.vaccinationState != KHC.VaccinationCertificate.VaccinationTooRecent ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.neutralTextColor
            font.bold: true
            visible: !isNaN(certificate.date.getTime())
        }
        QQC2.Label {
            text: certificate.disease
            Kirigami.FormData.label: i18n("Disease:")
            visible: certificate.disease
        }
        QQC2.Label {
            text: certificate.vaccineType
            Kirigami.FormData.label: i18n("Type:")
            visible: certificate.vaccineType
        }
        QQC2.Label {
            text: certificate.vaccineUrl != "" ? '<a href="' + certificate.vaccineUrl + '">' + certificate.vaccine + '</a>' : certificate.vaccine
            Kirigami.FormData.label: i18n("Vaccine:")
            onLinkActivated: Qt.openUrlExternally(link)
            visible: text !== ""
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: certificate.manufacturer
            Kirigami.FormData.label: i18n("Manufacturer:")
            visible: text !== ""
        }
        QQC2.Label {
            text: certificate.totalDoses > 0 ? i18n("%1/%2", certificate.dose, certificate.totalDoses) : certificate.dose
            Kirigami.FormData.label: i18n("Dose:")
            color: certificate.dose < certificate.totalDoses ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.positiveTextColor
            font.bold: true
            visible: certificate.dose > 0
        }
        QQC2.Label {
            readonly property var country: Country.fromAlpha2(certificate.country)
            text: country.emojiFlag + " " + country.name
            Kirigami.FormData.label: i18n("Country:")
            visible: certificate.country
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Certificate")
        }

        QQC2.Label {
            text: certificate.certificateIssuer
            Kirigami.FormData.label: i18n("Issuer:")
            visible: text !== ""
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: certificate.certificateId
            Kirigami.FormData.label: i18n("Identifier:")
            wrapMode: Text.Wrap
            visible: certificate.certificateId
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
