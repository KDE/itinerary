/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

FormCard.FormCardPage {
    id: root

    title: i18n("Program Membership")

    property string passId
    property var programMembership

    data: [
        Connections {
            target: PassManager
            function onPassChanged(passId) {
                if (passId == root.passId)
                    root.programMembership = PassManager.pass(passId);
            }
        },
        BarcodeScanModeButton {
            id: scanModeController
            page: root
            visible: barcodeContainer.visible
        },
        Kirigami.PromptDialog {
            id: deleteWarningDialog

            title: i18n("Delete Pass")
            subtitle: i18n("Do you really want to delete this pass?")

            standardButtons: QQC2.Dialog.Cancel

            customFooterActions: [
                Kirigami.Action {
                    text: i18n("Delete")
                    icon.name: "edit-delete"
                    onTriggered: {
                        PassManager.remove(passId)
                        applicationWindow().pageStack.pop();
                    }
                }
            ]
        }
    ]

    ColumnLayout {
        spacing: 0

        App.CardPageTitle {
            emojiIcon: "🎫"
            text: programMembership.programName
            Layout.topMargin: kirigami.Units.smallSpacing
        }

        FormCard.FormCard {
            App.BarcodeContainer {
                id: barcodeContainer
                Layout.alignment: Qt.AlignCenter
                Layout.fillWidth: true
                Layout.bottomMargin: Kirigami.Units.largeSpacing

                barcodeType: programMembership.tokenType
                barcodeContent: programMembership.tokenData
                onDoubleClicked: scanModeController.toggle()
            }
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                id: memberNameLabel
                text: i18n("Member")
                description: programMembership.member.name
                visible: programMembership.member.name !== ""
            }
            FormCard.FormDelegateSeparator {
                visible: memberNameLabel.visible
            }
            FormCard.FormTextDelegate {
                id: membershipNumberLabel
                text: i18n("Number")
                description: programMembership.membershipNumber
                visible: programMembership.membershipNumber !== ""
            }
            FormCard.FormDelegateSeparator {
                visible: membershipNumberLabel.visible
            }
            FormCard.FormTextDelegate {
                text: i18n("Valid from")
                description: Localizer.formatDateOrDateTimeLocal(programMembership, "validFrom")
                visible: description !== ""
            }
            FormCard.FormTextDelegate {
                text: i18n("Valid until")
                description: Localizer.formatDateOrDateTimeLocal(programMembership, "validUntil")
                visible: description !== ""
            }
        }

        App.DocumentsCard {
            documentIds: PassManager.documentIds(programMembership)
            onAddDocument: (file) => {
                ApplicationController.addDocumentToPass(root.passId, file);
                documentIds = Qt.binding(function() { return PassManager.documentIds(programMembership) });
            }
            onRemoveDocument: (docId) => { ApplicationController.removeDocumentFromPass(root.passId, docId); }
        }

        FormCard.FormHeader {
            title: i18n("Actions")
        }

        FormCard.FormCard {
            FormCard.FormButtonDelegate {
                icon.name: "document-edit"
                text: i18n("Edit")
                onClicked: applicationWindow().pageStack.push(programMembershipEditor, {passId: root.passId, programMembership: root.programMembership});
            }
            FormCard.FormButtonDelegate {
                icon.name: "edit-delete"
                text: i18n("Delete")
                onClicked: deleteWarningDialog.open()
            }
        }
    }
}
