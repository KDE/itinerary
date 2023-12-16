/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import Qt.labs.platform
import org.kde.kirigami as Kirigami
import org.kde.kitinerary
import org.kde.itinerary
import org.kde.kirigamiaddons.formcard as FormCard
import "." as App

ColumnLayout {
    id: root

    property alias documentIds: docsModel.documentIds
    property alias title: cardHeader.title

    signal addDocument(string file)
    signal removeDocument(string docId)

    spacing: 0

    DocumentsModel {
        id: docsModel
        documentManager: DocumentManager
    }

    FileDialog {
        id: addDialog
        title: i18n("Add Document")
        folder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        nameFilters: [i18n("All Files (*.*)")]
        onAccepted: root.addDocument(file)
    }

    Component {
        id: documentDelegate
        FormCard.AbstractFormDelegate {
            Layout.fillWidth: true
            onClicked: ApplicationController.openDocument(model.filePath);
            Accessible.onPressAction: clicked()
            Accessible.name: model.display
            contentItem: RowLayout {
                Kirigami.Icon {
                    source: model.decoration
                    width: height
                    height: Kirigami.Units.iconSizes.small
                }

                QQC2.Label {
                    text: model.display
                    Layout.fillWidth: true
                    elide: Text.ElideMiddle
                    Accessible.ignored: true
                }

                Kirigami.ActionToolBar {
                    actions: QQC2.Action {
                        icon.name: "edit-delete"
                        text: i18n("Delete Document")
                        onTriggered: {
                            deleteWarningDialog.docId = model.id;
                            deleteWarningDialog.open()
                        }
                    }
                }
            }
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
                    root.removeDocument(deleteWarningDialog.docId);
                    deleteWarningDialog.close()
                }
            }
        ]
    }

    FormCard.FormHeader {
        id: cardHeader
        title: i18n("Documents")
    }

    FormCard.FormCard {
        Repeater {
            delegate: documentDelegate
            model: docsModel
        }

        FormCard.FormTextDelegate {
            text: i18n("No documents attached to this reservation.")
            visible: docsModel.empty
            Accessible.ignored: !visible
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: i18n("Add Document...")
            onClicked: addDialog.open()
        }
    }
}
