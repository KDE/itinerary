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
import QtQuick.Dialogs 1.0
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.0 as Kirigami
import "." as App

Kirigami.ApplicationWindow {
    title: qsTr("KDE Itinerary")
    header: Kirigami.ApplicationHeader {}

    width: 480
    height: 720

    FileDialog {
        id: fileDialog
        title: qsTr("Please choose a file")
        folder: shortcuts.home
        onAccepted: {
            console.log(fileDialog.fileUrls);
            _pkpassManager.importPass(fileDialog.fileUrl);
        }
    }

    globalDrawer: Kirigami.GlobalDrawer {
        title: qsTr("KDE Itinerary")
        titleIcon: "map-symbolic"
        actions: [
            Kirigami.Action {
                text: qsTr("Import Pass...")
                iconName: "document-open"
                onTriggered: {
                    fileDialog.visible = true;
                }
            }
        ]
    }
    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }
    pageStack.initialPage: mainPageComponent
    Component {
        id: mainPageComponent
        App.TimelinePage {}
    }
}
