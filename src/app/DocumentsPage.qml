/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import Qt.labs.platform 1.1
import org.kde.kirigami 2.19 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import "." as App

MobileForm.FormCard {
    id: documentFormCard
    property var controller: null
    Layout.fillWidth: true
    Layout.topMargin: Kirigami.Units.largeSpacing

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
        MobileForm.AbstractFormDelegate {
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
                    ApplicationController.removeDocument(controller.batchId, deleteWarningDialog.docId);
                    deleteWarningDialog.close()
                }
            }
        ]
    }

    contentItem: ColumnLayout {
        spacing: 0

        MobileForm.FormCardHeader {
            title: i18n("Documents and Tickets")
        }

        Repeater {
            delegate: documentDelegate
            model: docsModel
        }

        MobileForm.FormTextDelegate {
            text: i18n("No documents attached to this reservation.")
            visible: docsModel.empty
            Accessible.ignored: !visible
        }

        MobileForm.FormDelegateSeparator {}

        MobileForm.FormButtonDelegate {
            text: i18n("Add Document...")
            onClicked: addDialog.open()
        }
    }
}
