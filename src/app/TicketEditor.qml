/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitinerary
import org.kde.itinerary

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
        spacing: 0

        FormCard.FormHeader {
            title: i18n("Ticket")
        }

        FormCard.FormCard {
            FormCard.FormTextFieldDelegate {
                id: ticketNameEdit
                label: i18n("Ticket name")
                text: ticket.name
                status: Kirigami.MessageType.Error
                statusMessage: text === "" ? i18n("Ticket name must not be empty.") : ""
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
