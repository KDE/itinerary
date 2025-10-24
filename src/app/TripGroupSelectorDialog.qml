/*
    SPDX-FileCopyrightText: 2025 Jonah Brüchert <jbb@kaidan.im>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import org.kde.kirigami as Kirigami
import QtQuick


Kirigami.Dialog {
    id: tripGroupDialog

    title: i18nc("@title:dialog", "Add Journey to Trip")

    signal tripGroupSelected(string tgId)

    property string suggestedName: ""

    preferredWidth: Kirigami.Units.gridUnit * 30

    standardButtons: Kirigami.Dialog.NoButton

    TripGroupEditorDialog {
        id: createTripDialog
        tripGroupName: suggestedName
        onAccepted: {
            const tgId = TripGroupManager.createEmptyGroup(createTripDialog.tripGroupName);
            tripGroupDialog.tripGroupSelected(tgId)
        }
    }

    ListView {
        reuseItems: true
        implicitHeight: 500

        model: TripGroupModel

        delegate: TripGroupDelegate {
            id: delegate

            width: parent.width

            onClicked: tripGroupDialog.tripGroupSelected(delegate.tripGroupId)
            Accessible.onPressAction: tripGroupDialog.tripGroupSelected(delegate.tripGroupId)
        }
    }

    customFooterActions: [
        Kirigami.Action {
            text: i18n("New Trip")
            icon.name: "list-add"

            onTriggered: {
                createTripDialog.open()
            }
        }
    ]
}
