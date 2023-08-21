/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    title: i18n("Ticket")
    property string passId
    property var ticket

    Connections {
        target: PassManager
        function onPassChanged(passId) {
            if (passId == root.passId)
                root.ticket = PassManager.pass(passId);
        }
    }

    Component {
        id: editor
        App.TicketEditor {}
    }

    data: BarcodeScanModeButton {
        page: root
        visible: barcodeContainer.visible
    }

    Kirigami.PromptDialog {
        id: deleteWarningDialog

        title: i18n("Delete Ticket")
        subtitle: i18n("Do you really want to delete this ticket?")

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
                    text: root.ticket.name
                    horizontalAlignment: Qt.AlignHCenter
                    font.bold: true
                }

                App.BarcodeContainer {
                    id: barcodeContainer
                    Layout.alignment: Qt.AlignCenter
                    Layout.fillWidth: true
                    Layout.bottomMargin: Kirigami.Units.largeSpacing

                    barcodeType: root.ticket.ticketTokenType
                    barcodeContent: root.ticket.ticketTokenData
                    onDoubleClicked: scanModeController.toggle()
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                MobileForm.FormCardHeader {
                    title: i18n("Ticket")
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Name")
                    description: ticket.underName.name
                    visible: ticket.underName.name !== ""
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Valid from")
                    description: Localizer.formatDateOrDateTimeLocal(ticket, "validFrom")
                    visible: description
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Valid until")
                    description: Localizer.formatDateOrDateTimeLocal(ticket, "validUntil")
                    visible: description
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Number")
                    description: ticket.ticketNumber
                    visible: ticket.ticketNumber !== ""
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Class")
                    description: ticket.ticketedSeat.seatingType
                    visible: ticket.ticketedSeat.seatingType !== ""
                }

                FormPriceDelegate {
                    item: root.ticket
                }
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
                        onTriggered: applicationWindow().pageStack.push(editor, {passId: root.passId, ticket: root.ticket});
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
