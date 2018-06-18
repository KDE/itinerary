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
import "." as App

Kirigami.ApplicationWindow {
    title: i18n("KDE Itinerary")
    header: Kirigami.ApplicationHeader {}

    width: 480
    height: 720

    App.ImportDialog {
        id: importDialog
    }

    globalDrawer: Kirigami.GlobalDrawer {
        title: i18n("KDE Itinerary")
        titleIcon: "map-symbolic"
        actions: [
            Kirigami.Action {
                text: i18n("Import Reservation...")
                iconName: "document-open"
                onTriggered: importDialog.importReservation()
            },
            Kirigami.Action {
                text: i18n("Import Pass...")
                iconName: "document-open"
                onTriggered: importDialog.importPass()
            },
            Kirigami.Action {
                text: i18n("Check for Updates")
                iconName: "view-refresh"
                onTriggered: {
                    _pkpassManager.updatePasses();
                }
            },
            Kirigami.Action {
                id: settingsAction
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
        App.SettingsPage {
            id: settingsPage
            Binding {
                target: settingsAction
                property: "enabled"
                value: !settingsPage.isCurrentPage
            }
        }
    }
}
