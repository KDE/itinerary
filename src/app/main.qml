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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.0 as Kirigami
import "." as App

Kirigami.ApplicationWindow {
    title: i18n("KDE Itinerary")
    reachableModeEnabled: false

    width: 480
    height: 720

    globalDrawer: Kirigami.GlobalDrawer {
        title: i18n("KDE Itinerary")
        titleIcon: "map-symbolic"
        actions: [
            Kirigami.Action {
                text: i18n("Import...")
                iconName: "document-open"
                onTriggered: _appController.showImportFileDialog();
            },
            Kirigami.Action {
                text: i18n("Paste")
                iconName: "edit-paste"
                onTriggered: _appController.importFromClipboard()
                enabled: _appController.hasClipboardContent
            },
            Kirigami.Action {
                text: i18n("Check Calendar")
                iconName: "view-calendar-day"
                onTriggered: _appController.checkCalendar()
                visible: Qt.platform.os == "android"
            },
            Kirigami.Action {
                text: i18n("Check for Updates")
                iconName: "view-refresh"
                onTriggered: {
                    _liveDataManager.checkForUpdates();
                }
            },
            Kirigami.Action {
                id: settingsAction
                text: i18n("Settings...")
                iconName: "settings-configure"
                onTriggered: pageStack.push(settingsComponent)
            },
            Kirigami.Action {
                text: i18n("Export...")
                iconName: "export-symbolic"
                onTriggered: _appController.exportData();
            },
            Kirigami.Action {
                id: aboutAction
                text: i18n("About")
                iconName: "help-about"
                onTriggered: pageStack.push(aboutComponent)
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
    Component {
        id: aboutComponent
        App.AboutPage {
            id: aboutPage
            Binding {
                target: aboutAction
                property: "enabled"
                value: !aboutPage.isCurrentPage
            }
        }
    }
}
