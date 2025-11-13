// SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.itinerary

/** Dialog to confirm a reservation deletion and/or select individual reservations in a batch to delete. */
Kirigami.Dialog {
    id: root

    property string description
    property alias batchId: root.incidenceModel.batchId

    /** Emitted when the entire batch has been deleted. */
    signal batchDeleted()

    standardButtons: QQC2.Dialog.Cancel

    readonly property TicketTokenModel incidenceModel: TicketTokenModel {
        reservationManager: ReservationManager
    }

    readonly property bool isMultiIncidence: listView.count > 1

    customFooterActions: [
        Kirigami.Action {
            text: i18n("Delete Selected")
            icon.name: "edit-delete"
            visible: root.isMultiIncidence
            enabled: incidenceModel.selectedReservationIds.length > 0 && incidenceModel.selectedReservationIds.length < listView.count
            onTriggered: {
                for (const resId of incidenceModel.selectedReservationIds) {
                    ReservationManager.removeReservation(resId);
                }
                root.close();
            }
        },
        Kirigami.Action {
            text: root.isMultiIncidence ? i18n("Delete All") : i18n("Delete")
            icon.name: "edit-delete"
            onTriggered: {
                ReservationManager.removeBatch(root.batchId);
                root.close();
                root.batchDeleted()
            }
        }
    ]

    contentItem: Kirigami.Padding {
        horizontalPadding: Kirigami.Units.largeSpacing
        contentItem: ColumnLayout {
            QQC2.Label {
                text: root.description
            }
            ListView {
                id: listView
                visible: root.isMultiIncidence
                model: root.incidenceModel

                Layout.fillHeight: true
                Layout.preferredHeight: contentHeight

                delegate: QQC2.CheckDelegate {
                    id: delegateRoot
                    required property var model
                    text: model.display
                    checkable: true
                    Layout.fillWidth: true
                    onToggled: model.selected = delegateRoot.checked
                }
            }
        }
    }
}
