/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.itinerary

PkPassPage {
    id: root
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
                    PassManager.remove(genericPassId);
                    root.QQC2.ApplicationWindow.window.pageStack.pop();
                }
            }
        ]
    }

    actions: [
        Kirigami.Action {
            icon.name: "edit-delete"
            text: i18n("Delete")
            onTriggered: deleteWarningDialog.open()
        }
    ]
}
