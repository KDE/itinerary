/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Templates 2.15 as T
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    title: i18n("Edit Ticket")

    required property string passId
    required property var ticket

    readonly property bool isValidInput: ticketNameEdit.text !== ''

    property T.Action saveAction: Kirigami.Action {
        text: i18n("Save")
        icon.name: "document-save"
        enabled: root.isValidInput
        onTriggered: {
            let newTicket = PassManager.pass(root.passId);
            newTicket.name = ticketNameEdit.text;

            PassManager.update(root.passId, newTicket);
            applicationWindow().pageStack.pop();
        }
    }

    ColumnLayout {
        width: parent.width

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                MobileForm.FormCardHeader {
                    title: i18n("Ticket")
                }

                MobileForm.FormTextFieldDelegate {
                    id: ticketNameEdit
                    label: i18n("Ticket name")
                    text: ticket.name
                    status: Kirigami.MessageType.Error
                    statusMessage: text === "" ? i18n("Ticket name must not be empty.") : ""
                }
            }
        }
    }

    footer: QQC2.ToolBar {
        contentItem: RowLayout {
            Item {
                Layout.fillWidth: true
            }

            QQC2.Button {
                action: root.saveAction
            }
        }
    }
}
