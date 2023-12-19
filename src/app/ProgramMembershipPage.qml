/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitinerary
import org.kde.itinerary

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

        CardPageTitle {
            emojiIcon: "🎫"
            text: programMembership.programName
            Layout.topMargin: Kirigami.Units.smallSpacing
        }

        FormCard.FormCard {
            visible: programMembership.token.length > 0
            BarcodeContainer {
                id: barcodeContainer
                Layout.alignment: Qt.AlignCenter
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.largeSpacing
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

                trailing: QQC2.ToolButton {
                    display: QQC2.AbstractButton.IconOnly
                    text: i18nc("@info:tooltip", "Copy to Clipboard")
                    icon.name: "edit-copy"
                    onClicked: {
                        Clipboard.saveText(programMembership.membershipNumber);
                        applicationWindow().showPassiveNotification(i18n("Program membership number copied to clipboard"));
                    }

                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.visible: hovered
                }
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

        DocumentsCard {
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
