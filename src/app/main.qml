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
    title: i18n("KDE Itinerary")
    header: Kirigami.ApplicationHeader {}

    width: 480
    height: 720

    FileDialog {
        property bool loadPass: false
        id: fileDialog
        title: i18n("Please choose a file")
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
        title: i18n("KDE Itinerary")
        titleIcon: "map-symbolic"
        actions: [
            Kirigami.Action {
                text: i18n("Import Reservation...")
                iconName: "document-open"
                onTriggered: {
                    fileDialog.loadPass = false;
                    fileDialog.visible = true;
                }
            },
            Kirigami.Action {
                text: i18n("Import Pass...")
                iconName: "document-open"
                onTriggered: {
                    fileDialog.loadPass = true;
                    fileDialog.visible = true;
                }
            },
            Kirigami.Action {
                text: i18n("Check for Updates")
                iconName: "view-refresh"
                onTriggered: {
                    _pkpassManager.updatePasses();
                }
            },
            Kirigami.Action {
                text: i18n("Settings...")
                iconName: "settings-configure"
                onTriggered: pageStack.push(settingsComponent)
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
    Component {
        id: pkpassComponent
        App.PkPassPage {
            pass: _pkpassManager.passObject(passId)
        }
    }
    Component {
        id: settingsComponent
        App.SettingsPage {}
    }
}
