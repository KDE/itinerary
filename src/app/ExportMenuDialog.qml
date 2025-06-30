// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtCore as QtCore
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import QtQml.Models
import org.kde.kirigami as Kirigami
import org.kde.itinerary

Kirigami.MenuDialog {
    id: root

    property alias supportsGpxExport: gpxAction.visible

    /** Basename of the suggested file name when exporting. */
    property string suggestedName
    /** Basename of the settings keys used for remembering the last used folders. */
    property string settingsKey

    /** Emitted when exporting to a file was requested. */
    signal exportToFile(path: string)
    /** Emitted when exporting to a GPX file was requested. */
    signal exportToGpxFile(path: string)
    /** Emitted when exporting to a KDE Connect device was requested. */
    signal exportToKDEConnect(deviceId: string)

    property list<QQC2.Action> _actions: [
        Kirigami.Action {
            text: i18n("As Itinerary file…")
            icon.name: "document-export-symbolic"
            onTriggered: {
                if (root.suggestedName)
                    fileExportDialog.currentFile = root.suggestedName + ".itinerary"
                fileExportDialog.open();
            }
        },
        Kirigami.Action {
            id: gpxAction
            text: i18n("As GPX file…")
            icon.name: "map-globe"
            onTriggered: {
                if (root.suggestedName)
                    gpxExportDialog.currentFile = root.suggestedName  + ".gpx"
                gpxExportDialog.open();
            }
        }
    ]
    actions: root._actions
    Instantiator {
        model: KDEConnectDeviceModel {
            id: deviceModel
        }
        delegate: Kirigami.Action {
            text: i18n("Send to %1", model.name)
            icon.name: "kdeconnect-tray"
            onTriggered: root.exportToKDEConnect(model.deviceId)
        }
        onObjectAdded: (index, object) => {
            console.log(object)
            root._actions.push(object);
        }
    }
    onVisibleChanged: {
        if (root.visible)
            deviceModel.refresh();
    }

    FileDialog {
        id: fileExportDialog
        fileMode: FileDialog.SaveFile
        title: root.title
        nameFilters: [i18n("Itinerary file (*.itinerary)")]
        onAccepted: {
            root.exportToFile(fileExportDialog.selectedFile);
            if (root.settingsKey)
                Settings.writeFileDialogFolder(root.settingsKey + "Export", fileExportDialog.selectedFile)
        }
        onVisibleChanged: {
            if (fileExportDialog.visible) {
                fileExportDialog.currentFolder = Settings.readFileDialogFolder(root.settingsKey + "Export", QtCore.StandardPaths.writableLocation(QtCore.StandardPaths.DocumentsLocation));
            }
        }
    }
    FileDialog {
        id: gpxExportDialog
        fileMode: FileDialog.SaveFile
        title: root.title
        currentFolder: QtCore.StandardPaths.writableLocation(QtCore.StandardPaths.DocumentsLocation)
        nameFilters: [i18n("GPX Files (*.gpx)")]
        onAccepted: {
            root.exportToGpxFile(gpxExportDialog.selectedFile);
            if (root.settingsKey)
                Settings.writeFileDialogFolder(root.settingsKey + "GpxExport", gpxExportDialog.selectedFile)
        }
        onVisibleChanged: {
            if (gpxExportDialog.visible) {
                gpxExportDialog.currentFolder = Settings.readFileDialogFolder(root.settingsKey + "GpxExport", QtCore.StandardPaths.writableLocation(QtCore.StandardPaths.DocumentsLocation));
            }
        }
    }
}
