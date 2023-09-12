// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import internal.org.kde.kcalendarcore as KCalendarCore
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitemmodels
import org.kde.kitinerary
import org.kde.itinerary
import "." as App

ColumnLayout {
    id: root

    spacing: 0

    property var batchId
    property var editor
    property var reservation
    property list<QQC2.Action> additionalActions

    readonly property string passId: PkPassManager.passId(root.reservation)

    property list<QQC2.Action> _defaultActions: [
        Kirigami.Action {
            icon.name: root.passId !== "" ? "image://org.kde.pkpass/" + passId + "/icon" : ""
            text: root.reservation.className === "FlightReservation" ? i18n("Show Boarding Pass") : i18n("Show Ticket")
            visible: PkPassManager.hasPass(root.passId)
            onTriggered: applicationWindow().pageStack.push(pkpassComponent, {"passId": root.passId });
        },
        Kirigami.Action {
            icon.name: "qrc:///images/transfer.svg"
            text: i18n("Add transfer before")
            enabled: TransferManager.canAddTransfer(root.batchId, Transfer.Before)
            onTriggered: {
                var t = TransferManager.addTransfer(root.batchId, Transfer.Before);
                applicationWindow().pageStack.push(transferPage, {"transfer": t });
            }
        },
        Kirigami.Action {
            icon.name: "qrc:///images/transfer.svg"
            text: i18n("Add transfer after")
            enabled: TransferManager.canAddTransfer(root.batchId, Transfer.After)
            onTriggered: {
                var t = TransferManager.addTransfer(root.batchId, Transfer.After);
                applicationWindow().pageStack.push(transferPage, {"transfer": t });
            }
        },
        Kirigami.Action {
            icon.name: "document-edit"
            text: i18n("Edit")
            visible: root.editor != undefined
            onTriggered: applicationWindow().pageStack.push(editor, {batchId: root.batchId});
        },
        Kirigami.Action {
            icon.name: "view-calendar-day"
            text: i18n("Add to calendar...")
            onTriggered: PermissionManager.requestPermission(Permission.WriteCalendar, function() {
                if (!writableCalendars.sourceModel) {
                    writableCalendars.sourceModel = calendarModel.createObject(root);
                }
                calendarSelector.open();
            })
            visible: KCalendarCore.CalendarPluginLoader.hasPlugin
        },
        Kirigami.Action {
            icon.name: "export-symbolic"
            text: i18n("Export...")
            onTriggered: exportBatchDialog.open()
        },
        Kirigami.Action {
            icon.name: "edit-delete"
            text: i18n("Delete")
            onTriggered: deleteWarningDialog.open()
        }
    ]

    Component {
        id: transferPage
        App.TransferPage {}
    }

    Component {
        id: calendarModel
        // needs to be created on demand, after we have calendar access permissions
        KCalendarCore.CalendarListModel {}
    }
    KSortFilterProxyModel {
        id: writableCalendars
        filterRoleName: "accessMode"
        filterString: KCalendarCore.KCalendarCore.ReadWrite
    }
    App.CalendarSelectionSheet {
        id: calendarSelector
        model: writableCalendars
        onCalendarSelected: controller.addToCalendar(calendar);
    }

    Component {
        id: pkpassComponent
        App.PkPassPage {
            pass: PkPassManager.pass(root.passId)
        }
    }

    Kirigami.PromptDialog {
        id: deleteWarningDialog

        title: i18n("Delete Event")
        subtitle: i18n("Do you really want to delete this event?")

        standardButtons: QQC2.Dialog.Cancel

        customFooterActions: [
            Kirigami.Action {
                text: i18n("Delete")
                icon.name: "edit-delete"
                onTriggered: {
                    ReservationManager.removeBatch(root.batchId);
                    applicationWindow().pageStack.pop();
                }
            }
        ]
    }

    Kirigami.MenuDialog {
        id: exportBatchDialog
        title: i18n("Export")
        property list<QQC2.Action> _actions: [
            Kirigami.Action {
                text: i18n("As Itinerary file...")
                icon.name: "export-symbolic"
                onTriggered: batchFileExportDialog.open()
            }
        ]
        actions: exportBatchDialog._actions
        Instantiator {
            model: KDEConnectDeviceModel {
                id: deviceModel
            }
            delegate: Kirigami.Action {
                text: i18n("Send to %1", model.name)
                icon.name: "kdeconnect-tray"
                onTriggered: ApplicationController.exportBatchToKDEConnect(root.batchId, model.deviceId)
            }
            onObjectAdded: exportBatchDialog._actions.push(object)
        }
        onVisibleChanged: {
            if (exportBatchDialog.visible)
                deviceModel.refresh();
        }
    }
    FileDialog {
        id: batchFileExportDialog
        fileMode: FileDialog.SaveFile
        title: i18n("Export Reservation")
        currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        nameFilters: [i18n("Itinerary file (*.itinerary)")]
        onAccepted: ApplicationController.exportBatchToFile(root.batchId, selectedFile)
    }

    FormCard.FormHeader {
        title: i18n("Actions")
    }

    FormCard.FormCard {
        Repeater {
            FormCard.FormButtonDelegate {
                action: modelData
                visible: modelData.visible ?? true
            }

            model: _defaultActions
        }

        // TODO this needs multi-traveler support!
        Repeater {
            model: root.reservation.potentialAction
            delegate: FormCard.FormButtonDelegate {
                text: {
                    if (modelData.className == "CancelAction") return i18nc("cancel as in revoking a booking", "Cancel Reservation");
                    if (modelData.className == "CheckInAction") return i18n("Check-in");
                    if (modelData.className == "DownloadAction") return i18n("Download");
                    if (modelData.className == "UpdateAction") return i18n("Change Reservation");
                    if (modelData.className == "ViewAction") return i18n("View Reservation");
                }
                icon.name: {
                    if (modelData.className == "CancelAction") return "dialog-cancel";
                    if (modelData.className == "CheckInAction") return "checkmark";
                    if (modelData.className == "DownloadAction") return "edit-download";
                    if (modelData.className == "UpdateAction") return "document-edit";
                    if (modelData.className == "ViewAction") return "document-open";
                }
                onClicked: Qt.openUrlExternally(modelData.target)
            }
        }

        Repeater {
            FormCard.FormButtonDelegate {
                action: modelData
                visible: modelData.visible ?? true
            }

            model: additionalActions
        }
    }
}
