/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.0 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    /** The reservation batch identifier (@see ReservationManager). */
    property alias batchId: _controller.batchId
    /** Currently selected reservation of the batch. */
    property var currentReservationId: batchId
    /** @deprecated */
    readonly property var reservation: _reservationManager.reservation(currentReservationId);
    /** Reservation::reservationFor, unique for all travelers on a multi-traveler reservation set */
    readonly property var reservationFor: reservation.reservationFor
    property var editor
    readonly property string passId: _pkpassManager.passId(reservation)

    readonly property QtObject controller: TimelineDelegateController {
        id: _controller
        reservationManager: _reservationManager
        liveDataManager: _liveDataManager
    }
    property alias arrival: _controller.arrival
    property alias departure: _controller.departure

    Kirigami.OverlaySheet {
        id: deleteWarningSheet

        QQC2.Label {
            text: i18n("Do you really want to delete this event?")
            wrapMode: Text.WordWrap
        }

        footer: RowLayout {
            QQC2.Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Delete")
                icon.name: "edit-delete"
                onClicked: {
                    _reservationManager.removeBatch(root.batchId);
                    applicationWindow().pageStack.pop();
                }
            }
        }
    }

    Component {
        id: pkpassComponent
        App.PkPassPage {
            pass: _pkpassManager.passObject(passId)
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
        model: reservation.potentialAction
        delegate: Component {
            Kirigami.Action {
                text: {
                    if (modelData.className == "CancelAction") return i18n("Cancel Reservation");
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
                text: i18n("Show Boarding Pass")
                visible: _pkpassManager.hasPass(root.passId)
                onTriggered: applicationWindow().pageStack.push(pkpassComponent, {"passId": root.passId });
            },
            Kirigami.Action {
                iconName: "folder-documents-symbolic"
                text: i18n("Documents (%1)", root.reservation.subjectOf.length)
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
                iconName: "edit-delete"
                text: i18n("Delete")
                onTriggered: deleteWarningSheet.sheetOpen = true
            }
        ]
    }
}
