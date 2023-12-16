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
import "." as App

FormCard.FormCardPage {
    id: root

    title: i18n("Ticket")

    property string passId
    property var ticket

    data: [
        Connections {
            target: PassManager
            function onPassChanged(passId) {
                if (passId == root.passId)
                    root.ticket = PassManager.pass(passId);
            }
        },

        BarcodeScanModeButton {
            page: root
            visible: barcodeContainer.visible
        },

        Kirigami.PromptDialog {
            id: deleteWarningDialog

            title: i18n("Delete Ticket")
            subtitle: i18n("Do you really want to delete this ticket?")
            parent: applicationWindow()

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
            text: root.ticket.name
        }

        FormCard.FormCard {
            App.BarcodeContainer {
                id: barcodeContainer
                Layout.alignment: Qt.AlignCenter
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.largeSpacing
                Layout.bottomMargin: Kirigami.Units.largeSpacing

                barcodeType: root.ticket.ticketTokenType
                barcodeContent: root.ticket.ticketTokenData
                onDoubleClicked: scanModeController.toggle()
            }
        }

        FormCard.FormHeader {
            title: i18n("Ticket")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                text: i18n("Name")
                description: ticket.underName.name
                visible: ticket.underName.name !== ""
            }

            FormCard.FormTextDelegate {
                text: i18n("Valid from")
                description: Localizer.formatDateOrDateTimeLocal(ticket, "validFrom")
                visible: description
            }

            FormCard.FormTextDelegate {
                text: i18n("Valid until")
                description: Localizer.formatDateOrDateTimeLocal(ticket, "validUntil")
                visible: description
            }

            FormCard.FormTextDelegate {
                text: i18n("Number")
                description: ticket.ticketNumber
                visible: ticket.ticketNumber !== ""
            }

            FormCard.FormTextDelegate {
                text: i18n("Class")
                description: ticket.ticketedSeat.seatingType
                visible: ticket.ticketedSeat.seatingType !== ""
            }

            FormPriceDelegate {
                item: root.ticket
            }
        }

        App.DocumentsCard {
            documentIds: PassManager.documentIds(ticket)
            onAddDocument: (file) => {
                ApplicationController.addDocumentToPass(root.passId, file);
                documentIds = Qt.binding(function() { return PassManager.documentIds(ticket) });
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
                onClicked: applicationWindow().pageStack.push(editor, {passId: root.passId, ticket: root.ticket});

                Component {
                    id: editor
                    App.TicketEditor {}
                }
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormButtonDelegate {
                icon.name: "edit-delete"
                text: i18n("Delete")
                onClicked: deleteWarningDialog.open()
            }
        }
    }
}
