/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: page
    title: i18n("Documents")
    property var controller: null

    DocumentsModel {
        id: docsModel
        reservationManager: _reservationManager
        batchId: controller.batchId
        documentManager: DocumentManager
    }

    Component {
        id: documentDelegate
        Kirigami.SwipeListItem {
            RowLayout {
                Kirigami.Icon {
                    source: model.decoration
                    width: height
                    height: Kirigami.Units.iconSizes.small
                }
                QQC2.Label {
                    text: model.display
                }
            }
            actions: [
                Kirigami.Action {
                    iconName: "document-open"
                    text: i18n("Open Document")
                    onTriggered: Qt.openUrlExternally(model.filePath);
                },
                Kirigami.Action {
                    iconName: "edit-delete"
                    text: i18n("Delete Document")
                    // TODO safety question
                    onTriggered: DocumentManager.removeDocument(model.id);
                }
            ]
        }
    }

    actions {
        contextualActions: [
            Kirigami.Action {
                iconName: "list-add"
                text: i18n("Add Document...")
                onTriggered: _appController.addDocument(controller.batchId)
            }
        ]
    }

    ListView {
        delegate: documentDelegate
        model: docsModel

        QQC2.Label {
            text: i18n("No documents attached to this reservation.")
            anchors.centerIn: parent
            visible: docsModel.empty
        }
    }
}
