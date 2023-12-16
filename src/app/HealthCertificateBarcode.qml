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
                    return i18n("Vaccination certificate of %1", name);
                case KHC.HealthCertificate.Test:
                    return i18n("Test certificate of %1", name);
                case KHC.HealthCertificate.Recovery:
                    return i18n("Recovery certificate of %1", name);
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
                if (!isNaN(certificate.certificateExpiryDate.getTime()) && certificate.certificateExpiryDate.getTime() < Date.now()) {
                    return i18n("Certificate expired");
                }
                if (certificate.validationState == KHC.HealthCertificate.Invalid) {
                    return i18n("Certificate invalid");
                }
                if (certificate.signatureState == KHC.HealthCertificate.UnknownSignature) {
                    return i18n("Unknown issuer certificate");
                }

                switch (certificate.type) {
                case KHC.HealthCertificate.Vaccination:
                    switch (certificate.vaccinationState) {
                        case KHC.VaccinationCertificate.VaccinationTooRecent: return i18n("Recently vaccinated");
                        case KHC.VaccinationCertificate.PartiallyVaccinated: return i18n("Partially vaccinated");
                        case KHC.VaccinationCertificate.Vaccinated: return i18n("Vaccinated");
                        case KHC.VaccinationCertificate.FullyVaccinated: return i18n("Fully vaccinated");
                    }
                    break;
                case KHC.HealthCertificate.Test:
                    if (certificate.isCurrent) {
                        switch (certificate.result)  {
                            case KHC.TestCertificate.Negative: return i18n("Negative test");
                            case KHC.TestCertificate.Positive: return i18n("Positive test");
                        }
                    }
                    return i18n("Test expired");
                case KHC.HealthCertificate.Recovery:
                    return i18n("Recovery certificate valid");
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
        color: {
            if (certificate.type == KHC.HealthCertificate.Test && certificate.result == KHC.TestCertificate.Positive) {
                return Kirigami.Theme.negativeTextColor;
            }
            switch (certificate.validationState) {
                case KHC.HealthCertificate.Valid:
                    return certificate.signatureState == KHC.HealthCertificate.UncheckedSignature ? 'transparent' : Kirigami.Theme.positiveTextColor;
                case KHC.HealthCertificate.Partial: return Kirigami.Theme.neutralTextColor;
                case KHC.HealthCertificate.Invalid: return Kirigami.Theme.negativeTextColor;
                default: return "transparent"
            }
        }
    }
}
