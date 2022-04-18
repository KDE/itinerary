/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.itinerary 1.0
import "." as App

PkPassPage {
    property string genericPassId

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
                    PassManager.remove(genericPassId)
                    applicationWindow().pageStack.pop();
                }
            }
        }
    }

    actions.contextualActions: [
        Kirigami.Action {
            icon.name: "edit-delete"
            text: i18n("Delete")
            onTriggered: deleteWarningSheet.open()
        }
    ]
}
