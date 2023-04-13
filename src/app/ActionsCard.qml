// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import internal.org.kde.kcalendarcore 1.0 as KCalendarCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kitemmodels 1.0
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

MobileForm.FormCard {
    id: root
    Layout.fillWidth: true
    Layout.topMargin: Kirigami.Units.largeSpacing

    property string passId
    property list<QQC2.Action> additionalActions

    property list<QQC2.Action> _defaultActions: [
        Kirigami.Action {
            icon.name: root.passId !== "" ? "image://org.kde.pkpass/" + passId + "/icon" : ""
            text: currentReservation.className === "FlightReservation" ? i18n("Show Boarding Pass") : i18n("Show Ticket")
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
            icon.name: "edit-delete"
            text: i18n("Delete")
            onTriggered: deleteWarningDialog.open()
        }
    ]

    property var batchId
    property var editor

    // TODO this needs multi-traveler support!
    Instantiator {
        model: currentReservation.potentialAction
        delegate: Component {
            Kirigami.Action {
                text: {
                    if (modelData.className == "CancelAction") return i18nc("cancel as in revoking a booking", "Cancel Reservation");
                    if (modelData.className == "CheckInAction") return i18n("Check-in");
                    if (modelData.className == "DownloadAction") return i18n("Download");
                    if (modelData.className == "UpdateAction") return i18n("Change Reservation");
                    if (modelData.className == "ViewAction") return i18n("View Reservation");
                }
                iconName: {
                    if (modelData.className == "CancelAction") return "dialog-cancel";
                    if (modelData.className == "CheckInAction") return "checkmark";
                    if (modelData.className == "DownloadAction") return "edit-download";
                    if (modelData.className == "UpdateAction") return "document-edit";
                    if (modelData.className == "ViewAction") return "document-open";
                }
                onTriggered: Qt.openUrlExternally(modelData.target)
            }
        }
        onObjectAdded: _defaultActions.push(object)
    }

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
        filterRole: "accessMode"
        filterString: KCalendarCore.KCalendarCore.ReadWrite
    }
    Kirigami.OverlaySheet {
        id: calendarSelector
        title: i18n("Select Calendar")

        ListView {
            model: writableCalendars
            delegate: Kirigami.BasicListItem {
                text: model.name
                onClicked: {
                    controller.addToCalendar(model.calendar);
                    calendarSelector.close();
                }
            }
        }
    }

    Component {
        id: pkpassComponent
        App.PkPassPage {
            pass: PkPassManager.pass(passId)
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

    contentItem: ColumnLayout {
        spacing: 0

        MobileForm.FormCardHeader {
            title: i18n("Actions")
        }

        Repeater {
            MobileForm.FormButtonDelegate {
                action: modelData
                visible: modelData.visible ?? true
            }

            model: _defaultActions
        }

        Repeater {
            MobileForm.FormButtonDelegate {
                action: modelData
                visible: modelData.visible ?? true
            }

            model: additionalActions
        }
    }
}
