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
        property bool loadPass: false
        id: fileDialog
        title: qsTr("Please choose a file")
        folder: shortcuts.home
        onAccepted: {
            console.log(fileDialog.fileUrls);
            if (loadPass)
                _pkpassManager.importPass(fileDialog.fileUrl);
            else
                _reservationManager.importReservation(fileDialog.fileUrl);
        }
    }

    globalDrawer: Kirigami.GlobalDrawer {
        title: qsTr("KDE Itinerary")
        titleIcon: "map-symbolic"
        actions: [
            Kirigami.Action {
                text: qsTr("Import Reservation...")
                iconName: "document-open"
                onTriggered: {
                    fileDialog.loadPass = false;
                    fileDialog.visible = true;
                }
            },
            Kirigami.Action {
                text: qsTr("Import Pass...")
                iconName: "document-open"
                onTriggered: {
                    fileDialog.loadPass = true;
                    fileDialog.visible = true;
                }
            },
            Kirigami.Action {
                text: qsTr("Check for Updates")
                iconName: "view-refresh"
                onTriggered: {
                    _pkpassManager.updatePasses();
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
        App.TimelinePage {
            onShowBoardingPass: {
                pageStack.push(pkpassComponent);
                pageStack.currentItem.passId = passId
                pageStack.currentItem.pass = pass
            }
        }
    }
    Component {
        id: pkpassComponent
        Kirigami.Page {
            property alias passId: pkpass.passId
            property alias pass: pkpass.pass
            App.BoardingPass {
                x: (parent.width - implicitWidth) / 2
                id: pkpass
            }

            onBackRequested: pageStack.pop()
        }
    }
}
