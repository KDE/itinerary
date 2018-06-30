/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.0 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    property string resId
    property variant reservation
    property string passId
    property variant editor

    Kirigami.OverlaySheet {
        id: deleteWarningSheet

        QQC2.Label {
            text: i18n("Do you really want to delete this event?")
            wrapMode: Text.WordWrap
        }

        footer: RowLayout {
            QQC2.Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Delete")
                icon.name: "edit-delete"
                onClicked: {
                    _reservationManager.removeReservation(root.resId)
                    applicationWindow().pageStack.pop()
                }
            }
        }
    }

    actions {
        contextualActions: [
            Kirigami.Action {
                iconSource: root.passId !== "" ? "image://org.kde.pkpass/" + passId + "/icon" : ""
                text: i18n("Show Boarding Pass")
                visible: root.passId !== ""
                onTriggered: applicationWindow().pageStack.push(pkpassComponent, {"passId": root.passId });
            },
            Kirigami.Action {
                iconName: "document-edit"
                text: i18n("Edit")
                visible: root.editor != undefined
                onTriggered: applicationWindow().pageStack.push(editor);
            },
            Kirigami.Action {
                iconName: "edit-delete"
                text: i18n("Delete")
                onTriggered: deleteWarningSheet.sheetOpen = true
            }
        ]
    }

    onBackRequested: pageStack.pop()
}
