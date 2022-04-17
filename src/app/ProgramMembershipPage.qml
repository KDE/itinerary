/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.19 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    title: i18n("Program Membership")
    property string passId
    property var programMembership

    BarcodeScanModeController {
        id: scanModeController
        page: root
    }

    Kirigami.OverlaySheet {
        id: deleteWarningSheet
        header: Kirigami.Heading {
            text: i18n("Delete Pass")
        }

        QQC2.Label {
            text: i18n("Do you really want to delete this pass?")
            wrapMode: Text.WordWrap
        }

        footer: RowLayout {
            QQC2.Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Delete")
                icon.name: "edit-delete"
                onClicked: {
                    PassManager.remove(passId)
                    applicationWindow().pageStack.pop();
                }
            }
        }
    }

    actions.main: Kirigami.Action {
        icon.name: "view-barcode-qr"
        text: i18n("Barcode Scan Mode")
        onTriggered: scanModeController.toggle()
        visible: barcodeContainer.visible
        checkable: true
        checked: scanModeController.enabled
    }

    actions.contextualActions: [
        Kirigami.Action {
            icon.name: "edit-delete"
            text: i18n("Delete")
            onTriggered: deleteWarningSheet.open()
        }
    ]

    ColumnLayout {
        width: parent.width

        QQC2.Label {
            Layout.fillWidth: true
            text: programMembership.programName
            horizontalAlignment: Qt.AlignHCenter
            font.bold: true
        }

        App.BarcodeContainer {
            id: barcodeContainer
            Layout.alignment: Qt.AlignCenter
            Layout.fillWidth: true

            barcodeType: programMembership.tokenType
            barcodeContent: programMembership.tokenData
            onDoubleClicked: scanModeController.toggle()
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true

            Kirigami.Separator {
                Kirigami.FormData.label: i18n("Member")
                Kirigami.FormData.isSection: true
                visible: nameLabel.visible || numberLabel.visible
            }
            QQC2.Label {
                id: nameLabel
                Kirigami.FormData.label: i18n("Name:")
                text: programMembership.member.name
                visible: programMembership.member.name !== ""
            }
            QQC2.Label {
                id: numberLabel
                Kirigami.FormData.label: i18n("Number:")
                text: programMembership.membershipNumber
                visible: programMembership.membershipNumber !== ""
            }
        }
    }
}
