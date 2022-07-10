/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.19 as Kirigami
import org.kde.itinerary 1.0
import "." as App

PkPassPage {
    property string genericPassId

    Kirigami.PromptDialog {
        id: deleteWarningDialog

        title: i18n("Delete Pass")
        subtitle: i18n("Do you really want to delete this pass?")

        standardButtons: QQC2.Dialog.Close
        customFooterActions: [
            Kirigami.Action {
                text: i18n("Delete")
                icon.name: "edit-delete"
                onTriggered: {
                    PassManager.remove(genericPassId)
                    applicationWindow().pageStack.pop();
                }
            }
        ]
    }

    actions.contextualActions: [
        Kirigami.Action {
            icon.name: "edit-delete"
            text: i18n("Delete")
            onTriggered: deleteWarningDialog.open()
        }
    ]
}
