/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kosmindoormap
import org.kde.osm.editorcontroller
import org.kde.itinerary

OSMElementInformationDialog {
    id: elementDetailsSheet
    customFooterActions: [
        Kirigami.Action {
            visible: Settings.osmContributorMode
            icon.name: "document-edit"
            text: i18n("Edit with iD")
            onTriggered: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.ID)
        },
        Kirigami.Action {
            visible: Settings.osmContributorMode && EditorController.hasEditor(Editor.JOSM)
            icon.name: "org.openstreetmap.josm"
            text: i18n("Edit with JOSM")
            onTriggered: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.JOSM)
        },
        Kirigami.Action {
            visible: Settings.osmContributorMode && EditorController.hasEditor(Editor.Vespucci)
            icon.name: "document-edit"
            text: i18n("Edit with Vespucci")
            onTriggered: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.Vespucci)
        }
    ]
}
