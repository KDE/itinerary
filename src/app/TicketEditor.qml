/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitinerary
import org.kde.itinerary

Kirigami.ScrollablePage {
    id: root
    title: i18n("Edit Ticket")

    required property string passId
    required property var ticket
    required property var pageStack

    readonly property bool isValidInput: ticketNameEdit.text !== ''

    property T.Action saveAction: Kirigami.Action {
        text: i18n("Save")
        icon.name: "document-save"
        enabled: root.isValidInput
        onTriggered: {
            let newTicket = PassManager.pass(root.passId);
            newTicket.name = ticketNameEdit.text;

            let underName = newTicket.underName ?? Factory.makePerson();
            underName.name = underNameEdit.text;
            newTicket.underName = underName;

            PassManager.update(root.passId, newTicket);
            root.pageStack.pop();
        }
    }

    data: FloatingButton {
        anchors {
            right: parent.right
            rightMargin: Kirigami.Units.largeSpacing + (root.contentItem.QQC2.ScrollBar && root.contentItem.QQC2.ScrollBar.vertical ? root.contentItem.QQC2.ScrollBar.vertical.width : 0)
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.largeSpacing
        }
        action: root.saveAction
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
            FormCard.FormTextFieldDelegate {
                id: underNameEdit
                label: i18n("Under name")
                text: ticket.underName.name
            }
        }

        }
    }
}
