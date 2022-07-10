/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.19 as Kirigami
import org.kde.kitemmodels 1.0
import internal.org.kde.kcalendarcore 1.0 as KCalendarCore
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    /** The reservation batch identifier (@see ReservationManager). */
    property alias batchId: _controller.batchId
    /** Currently selected reservation id of the batch. */
    property var currentReservationId: batchId
    /** Currently selected reservation of the batch. */
    readonly property var currentReservation: ReservationManager.reservation(currentReservationId);
    /** @deprecated */
    readonly property var reservation: ReservationManager.reservation(currentReservationId);
    /** Reservation::reservationFor, unique for all travelers on a multi-traveler reservation set */
    readonly property var reservationFor: reservation.reservationFor
    property var editor
    readonly property string passId: PkPassManager.passId(currentReservation)

    property QtObject controller: TimelineDelegateController {
        id: _controller
        reservationManager: ReservationManager
        liveDataManager: LiveDataManager
        transferManager: TransferManager
    }
    property alias arrival: _controller.arrival
    property alias departure: _controller.departure

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

    Component {
        id: docsComponent
        App.DocumentsPage {
        }
    }

    Component {
        id: transferPage
        App.TransferPage {}
    }

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
        onObjectAdded: actions.contextualActions.push(object)
    }

    actions {
        contextualActions: [
            Kirigami.Action {
                iconSource: root.passId !== "" ? "image://org.kde.pkpass/" + passId + "/icon" : ""
                text: currentReservation.className === "FlightReservation" ? i18n("Show Boarding Pass") : i18n("Show Ticket")
                visible: PkPassManager.hasPass(root.passId)
                onTriggered: applicationWindow().pageStack.push(pkpassComponent, {"passId": root.passId });
            },
            Kirigami.Action {
                iconName: "folder-documents-symbolic"
                text: i18n("Documents (%1)", _controller.documentCount)
                onTriggered: applicationWindow().pageStack.push(docsComponent, {"controller": root.controller });
            },
            Kirigami.Action {
                iconName: "qrc:///images/transfer.svg"
                text: i18n("Add transfer before")
                enabled: TransferManager.canAddTransfer(root.batchId, Transfer.Before)
                onTriggered: {
                    var t = TransferManager.addTransfer(root.batchId, Transfer.Before);
                    applicationWindow().pageStack.push(transferPage, {"transfer": t });
                }
            },
            Kirigami.Action {
                iconName: "qrc:///images/transfer.svg"
                text: i18n("Add transfer after")
                enabled: TransferManager.canAddTransfer(root.batchId, Transfer.After)
                onTriggered: {
                    var t = TransferManager.addTransfer(root.batchId, Transfer.After);
                    applicationWindow().pageStack.push(transferPage, {"transfer": t });
                }
            },
            Kirigami.Action {
                iconName: "document-edit"
                text: i18n("Edit")
                visible: root.editor != undefined
                onTriggered: applicationWindow().pageStack.push(editor);
            },
            Kirigami.Action {
                iconName: "view-calendar-day"
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
                iconName: "edit-delete"
                text: i18n("Delete")
                onTriggered: deleteWarningDialog.open()
            }
        ]
    }
}
