// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtCore as QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import QtQuick.Dialogs
import QtLocation as QtLocation
import org.kde.kirigami as Kirigami
import org.kde.solidextras as Solid
import org.kde.calendarcore as KCalendarCore
import org.kde.itinerary

Kirigami.Action {
    id: root

    required property Kirigami.PageRow pageStack
    readonly property list<T.Action> passImportActions: [openFileAction, clipboardAction, barcodeAction]

    text: i18n("Import")
    icon.name: "document-import-symbolic"

    Kirigami.Action {
        id: openFileAction

        text: i18nc("@action:inmenu", "From file")
        icon.name: "document-open"
        shortcut: StandardKey.Open
        onTriggered: {
            importFileDialog.open();
        }
        readonly property FileDialog importFileDialog: FileDialog {
            fileMode: FileDialog.OpenFile
            title: i18n("Import Reservation")
            currentFolder: QtCore.StandardPaths.writableLocation(QtCore.StandardPaths.DocumentsLocation)
            // Android has no file type selector, we get the superset of all filters there since Qt6 (apart from "all"),
            // so don't set any filters on Android in order to be able to open everything we can read
            nameFilters:  Qt.platform.os === "android" ?
                [i18n("All Files (*.*)")] :
                [i18n("All Files (*.*)"), i18n("PkPass files (*.pkpass)"), i18n("PDF files (*.pdf)"), i18n("iCal events (*.ics)"), i18n("KDE Itinerary files (*.itinerary)")]
            onAccepted: ImportController.importFromUrl(selectedFile)
        }
    }

    Kirigami.Action {
        id: clipboardAction

        text: i18nc("@action:inmenu", "From clipboard")
        icon.name: "edit-paste"
        enabled: Clipboard.hasText || Clipboard.hasUrls || Clipboard.hasBinaryData
        shortcut: StandardKey.Paste
        onTriggered: ImportController.importFromClipboard()
    }

    Kirigami.Action {
        id: barcodeAction

        text: i18nc("@action:inmenu", "From barcode")
        icon.name: "view-barcode-qr"
        onTriggered: {
            root.pageStack.layers.push(scanBarcodeComponent);
        }

        readonly property Component scanBarcodeComponent: BarcodeScannerPage {
            onBarcodeDetected: (result) => {
                const prevCount = ImportController.count;
                if (result.hasText) {
                    ImportController.importText(result.text);
                } else if (result.hasBinaryData) {
                    ImportController.importData(result.binaryData);
                }
                if (ImportController.count != prevCount) {
                    applicationWindow().pageStack.goBack();
                }
            }
        }
    }

    Kirigami.Action {
        icon.name: "view-calendar-day"
        text: i18nc("@action:inmenu", "From calendar")
        onTriggered: {
            PermissionManager.requestPermission(Permission.ReadCalendar, function() {
                if (!calendarSelector.model) {
                    // needs to be created on demand, after we have calendar access permissions
                    calendarSelector.model = Qt.createComponent("org.kde.calendarcore", "CalendarListModel").createObject(root);
                }
                calendarSelector.open();
            })
        }
        visible: KCalendarCore.CalendarPluginLoader.hasPlugin

        readonly property CalendarSelectionDialog calendarSelector: CalendarSelectionDialog {
            // parent: root.Overlay.overlay
            onCalendarSelected: (calendar) => {
                ImportController.enableAutoCommit = false;
                ImportController.importFromCalendar(calendar);
            }
        }
    }

    // TODO this should not be hardcoded here, but dynamically filled based on what online ticket
    // sources we support
    Kirigami.Action {
        text: i18nc("@action:inmenu Name of the train company in Germany", "From Deutsche Bahn reservation number")
        icon.name: "download"
        onTriggered: {
            root.pageStack.layers.push(Qt.createComponent("org.kde.itinerary", "OnlineImportPage"), {
                source: "db",
            });
        }
        visible: false // FIXME API endpoint is no longer responding
    }

    Kirigami.Action {
        text: i18nc("Name of the train company in France", "From SNCF reservation number")
        icon.name: "download"
        onTriggered: {
            root.pageStack.layers.push(Qt.createComponent("org.kde.itinerary", "OnlineImportPage"), {
                source: "sncf"
            });
        }
    }
}
