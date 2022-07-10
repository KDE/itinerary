/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import Qt.labs.platform 1.1
import org.kde.kirigami 2.19 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: page
    title: i18n("Documents")
    property var controller: null

    DocumentsModel {
        id: docsModel
        reservationManager: ReservationManager
        batchId: controller.batchId
        documentManager: DocumentManager
    }

    FileDialog {
        id: addDialog
        title: i18n("Add Document")
        folder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        nameFilters: [i18n("All Files (*.*)")]
        onAccepted: ApplicationController.addDocument(controller.batchId, file)
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
                    Layout.fillWidth: true
                    elide: Text.ElideMiddle
                }
            }
            actions: [
                Kirigami.Action {
                    iconName: "document-open"
                    text: i18n("Open Document")
                    onTriggered: ApplicationController.openDocument(model.filePath);
                },
                Kirigami.Action {
                    iconName: "edit-delete"
                    text: i18n("Delete Document")
                    onTriggered: {
                        deleteWarningDialog.docId = model.id;
                        deleteWarningDialog.open()
                    }
                }
            ]
        }
    }

    Kirigami.PromptDialog {
        id: deleteWarningDialog
        property string docId

        title: i18n("Delete Document")
        subtitle: i18n("Do you really want to delete this document?")

        standardButtons: QQC2.Dialog.Cancel

        customFooterActions: [
            Kirigami.Action {
                text: i18n("Delete")
                icon.name: "edit-delete"
                onTriggered: {
                    ApplicationController.removeDocument(controller.batchId, deleteWarningDialog.docId);
                    deleteWarningDialog.close()
                }
            }
        ]
    }

    actions {
        contextualActions: [
            Kirigami.Action {
                iconName: "list-add"
                text: i18n("Add Document...")
                onTriggered: addDialog.open()
            }
        ]
    }

    ListView {
        delegate: documentDelegate
        model: docsModel

        Kirigami.PlaceholderMessage {
            text: i18n("No documents attached to this reservation.")
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            visible: docsModel.empty
        }
    }
}
