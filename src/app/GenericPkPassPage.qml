/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtCore as QtCore
import QtQuick
import QtQuick.Dialogs
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.itinerary

PkPassPage {
    id: root
    property string genericPassId

    FileDialog {
        id: saveAsDialog
        fileMode: FileDialog.SaveFile
        title: i18n("Save Pass")
        nameFilters: [i18n("Apple Wallet pass (*.pkpass)")]
        onAccepted: {
            ApplicationController.exportPkPassToFile(root.passId, saveAsDialog.selectedFile);
            Settings.writeFileDialogFolder("pkPassSaveAs", saveAsDialog.selectedFile);
        }
        onVisibleChanged: {
            if (saveAsDialog.visible) {
                saveAsDialog.currentFolder = Settings.readFileDialogFolder("pkPassSaveAs", QtCore.StandardPaths.writableLocation(QtCore.StandardPaths.DocumentsLocation));
            }
        }
    }

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
            icon.name: "document-save-as-symbolic"
            text: i18n("Save As…")
            onTriggered: {
                saveAsDialog.currentFile = Util.slugify(root.pass.description !== "" ? root.pass.description : root.pass.logoText) + ".pkpass";
                saveAsDialog.open();
            }
        },
        Kirigami.Action {
            icon.name: "edit-delete"
            text: i18n("Delete")
            onTriggered: deleteWarningDialog.open()
        }
    ]
}
