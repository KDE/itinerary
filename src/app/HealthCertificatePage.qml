/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.khealthcertificate 1.0 as KHC
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    title: i18n("Health Certificates")

    actions {
        main: Kirigami.Action {
            icon.name: "view-barcode-qr"
            text: i18n("Barcode Scan Mode")
            onTriggered: scanModeController.toggle()
            visible: certSelector.currentValue != undefined
            checkable: true
            checked: scanModeController.enabled
        }
        contextualActions: [
            Kirigami.Action {
                iconName: "edit-paste"
                text: i18n("Import from Clipboard")
                onTriggered: ApplicationController.importFromClipboard()
            },
            Kirigami.Action {
                iconName: "edit-delete"
                text: i18n("Delete")
                onTriggered: deleteWarningSheet.open()
                enabled: certSelector.currentIndex >= 0 && certSelector.count > 0
            }
        ]
    }

    Kirigami.OverlaySheet {
        id: deleteWarningSheet
        QQC2.Label {
            text: i18n("Do you really want to delete this certificate?")
            wrapMode: Text.WordWrap
        }

        footer: RowLayout {
            QQC2.Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Delete")
                icon.name: "edit-delete"
                onClicked: {
                    HealthCertificateManager.removeCertificate(certSelector.currentIndex);
                    deleteWarningSheet.close();
                }
            }
        }
    }
    header: Item {
        height: childrenRect.height + 2 * Kirigami.Units.largeSpacing
        width: parent.width
        visible: certSelector.count > 0
        ColumnLayout {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Kirigami.Units.largeSpacing

            QQC2.ComboBox {
                id: certSelector
                Layout.fillWidth: true
                model: HealthCertificateManager
                valueRole: "certificate"
                textRole: "display"
            }
        }
    }

    Component {
        id: vaccinationDetails
        App.HealthCertificateVaccination {
            certificate: certSelector.currentValue
        }
    }
    Component {
        id: testDetails
        App.HealthCertificateTest {
            certificate: certSelector.currentValue
        }
    }
    Component {
        id: recoveryDetails
        App.HealthCertificateRecovery {
            certificate: certSelector.currentValue
        }
    }
    Component {
        id: helpText
        QQC2.Label {
            text: i18n("<p>You can import the following health certificates by scanning them with a barcode scanner app:<ul><li>European \"Digital Green Certificates\" for vaccinations, tests or recovery.</li><li>Indian vaccination certificates.</li></p>")
            wrapMode: Text.WordWrap
        }
    }

    Loader {
        id: loader
        sourceComponent: {
            if (certSelector.count == 0 || !certSelector.currentValue)
                return helpText;
            switch (certSelector.currentValue.type) {
                case KHC.HealthCertificate.Vaccination:
                    return vaccinationDetails;
                case KHC.HealthCertificate.Test:
                    return testDetails;
                case KHC.HealthCertificate.Recovery:
                    return recoveryDetails;
            }
            return helpText;
        }

        BarcodeScanModeController {
            id: scanModeController
            page: root
        }
        Connections {
            target: loader.item
            function onScanModeToggled() {
                scanModeController.toggle();
            }
        }
    }

}
