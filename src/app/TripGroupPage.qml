// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.components as Components
import org.kde.itinerary

Kirigami.ScrollablePage {
    id: root

    required property var tripGroup

    title: tripGroup.name

    actions: [
        T.Action {
            text: i18nc("@action:intoolbar", "Edit trip")
            icon.name: 'document-edit-symbolic'
            onTriggered: {
                const editorComponent = Qt.createComponent('org.kde.itinerary', 'TripGroupEditorDialog');
                const editor = editorComponent.createObject(applicationWindow(), {
                    mode: TripGroupEditorDialog.Edit,
                    tripGroup: root.tripGroup,
                });
                editor.open();
            }

        }
    ]

    ListView {
        model: tripGroup.elements

        delegate: Delegates.RoundedItemDelegate {
            id: delegate

            required property int index
            required property string modelData

            text: modelData
        }
    }
}
