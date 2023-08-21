/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    title: i18n("Program Membership")
    property string passId
    property var programMembership

    Connections {
        target: PassManager
        function onPassChanged(passId) {
            if (passId == root.passId)
                root.programMembership = PassManager.pass(passId);
        }
    }

    data: BarcodeScanModeButton {
        id: scanModeController
        page: root
        visible: barcodeContainer.visible
    }

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

    ColumnLayout {
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                Kirigami.Heading {
                    Layout.fillWidth: true
                    Layout.topMargin: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing
                    text: programMembership.programName
                    horizontalAlignment: Qt.AlignHCenter
                    font.bold: true
                }

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
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                MobileForm.FormTextDelegate {
                    id: memberNameLabel
                    text: i18n("Member")
                    description: programMembership.member.name
                    visible: programMembership.member.name !== ""
                }
                MobileForm.FormDelegateSeparator {
                    visible: memberNameLabel.visible
                }
                MobileForm.FormTextDelegate {
                    id: membershipNumberLabel
                    text: i18n("Number")
                    description: programMembership.membershipNumber
                    visible: programMembership.membershipNumber !== ""
                }
                MobileForm.FormDelegateSeparator {
                    visible: membershipNumberLabel.visible
                }
                MobileForm.FormTextDelegate {
                    text: i18n("Valid from")
                    description: Localizer.formatDateOrDateTimeLocal(programMembership, "validFrom")
                    visible: description !== ""
                }
                MobileForm.FormTextDelegate {
                    text: i18n("Valid until")
                    description: Localizer.formatDateOrDateTimeLocal(programMembership, "validUntil")
                    visible: description !== ""
                }
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

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                MobileForm.FormCardHeader {
                    title: i18n("Actions")
                }

                MobileForm.FormButtonDelegate {
                    action: Kirigami.Action {
                        icon.name: "document-edit"
                        text: i18n("Edit")
                        onTriggered: applicationWindow().pageStack.push(programMembershipEditor, {passId: root.passId, programMembership: root.programMembership});
                    }
                }
                MobileForm.FormButtonDelegate {
                    action: Kirigami.Action {
                        icon.name: "edit-delete"
                        text: i18n("Delete")
                        onTriggered: deleteWarningDialog.open()
                    }
                }
            }
        }
    }
}
