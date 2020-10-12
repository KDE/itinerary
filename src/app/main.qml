/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.13
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import Qt.labs.platform 1.1
import org.kde.kirigami 2.4 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.ApplicationWindow {
    title: i18n("KDE Itinerary")
    reachableModeEnabled: false

    width: 480
    height: 720

    FileDialog {
        id: fileDialog
        title: i18n("Import Reservation")
        folder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        nameFilters: [i18n("All Files (*.*)"), i18n("PkPass files (*.pkpass)"), i18n("PDF files (*.pdf)"), i18n("KDE Itinerary files (*.itinerary)")]
        onAccepted: ApplicationController.importFromUrl(file)
    }

    globalDrawer: Kirigami.GlobalDrawer {
        title: i18n("KDE Itinerary")
        titleIcon: "map-symbolic"
        isMenu: true
        actions: [
            Kirigami.Action {
                text: i18n("Import...")
                iconName: "document-open"
                onTriggered: fileDialog.open()
            },
            Kirigami.Action {
                text: i18n("Paste")
                iconName: "edit-paste"
                onTriggered: ApplicationController.importFromClipboard()
                enabled: ApplicationController.hasClipboardContent
            },
            Kirigami.Action {
                text: i18n("Check Calendar")
                iconName: "view-calendar-day"
                onTriggered: ApplicationController.checkCalendar()
                visible: Qt.platform.os == "android"
            },
            Kirigami.Action {
                text: i18n("Check for Updates")
                iconName: "view-refresh"
                onTriggered: {
                    LiveDataManager.checkForUpdates();
                }
            },
            /*Kirigami.Action {
                text: i18n("Download Maps")
                iconName: "download"
                onTriggered: MapDownloadManager.download();
            },*/
            Kirigami.Action {
                id: statsAction
                text: i18n("Statistics")
                iconName: "view-statistics"
                onTriggered: pageStack.push(statisticsComponent)
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
                onTriggered: ApplicationController.exportData();
            },
            Kirigami.Action {
                text: i18n("Help")
                iconName: "help-contents"
                onTriggered: pageStack.push(welcomeComponent)
            },
            Kirigami.Action {
                id: aboutAction
                text: i18n("About")
                iconName: "help-about-symbolic"
                onTriggered: pageStack.push(aboutComponent)
            },
            Kirigami.Action {
                id: devModeAction
                text: "Development"
                iconName: "tools-report-bug"
                onTriggered: pageStack.push(devModePageComponent)
                visible: Settings.developmentMode
            }
        ]
    }
    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }
    pageStack.initialPage: mainPageComponent

    DropArea {
        id: topDropArea
        onEntered: {
            if (!drag.hasUrls) {
                drag.accepted = false;
            }
        }
        onDropped: {
            for (var i in drop.urls) {
                ApplicationController.importFromUrl(drop.urls[i]);
            }
        }
        anchors.fill: parent
    }

    footer: Kirigami.InlineMessage {
        id: infoMessage
        Layout.fillWidth: true
        visible: false
        showCloseButton: true

        Connections {
            target: ReservationManager
            onInfoMessage: {
                infoMessage.text = msg;
                infoMessage.visible = true;
            }
        }
    }

    Component.onCompleted: {
        if (ReservationManager.isEmpty()) {
            pageStack.push(welcomeComponent);
        }
    }

    Component {
        id: mainPageComponent
        App.TimelinePage {}
    }
    Component {
        id: settingsComponent
        App.SettingsPage {
            id: settingsPage
            onIsCurrentPageChanged: settingsAction.enabled = !settingsPage.isCurrentPage
        }
    }
    Component {
        id: aboutComponent
        App.AboutPage {
            id: aboutPage
            onIsCurrentPageChanged: aboutAction.enabled = !aboutPage.isCurrentPage
        }
    }
    Component {
        id: statisticsComponent
        App.StatisticsPage {
            id: statsPage
            reservationManager: ReservationManager
            tripGroupManager: TripGroupManager
            onIsCurrentPageChanged: statsAction.enabled = !statsPage.isCurrentPage
        }
    }
    Component {
        id: welcomeComponent
        App.WelcomePage {}
    }
    Component {
        id: journeySectionPage
        App.JourneySectionPage {}
    }
    Component {
        id: indoorMapPage
        App.IndoorMapPage {}
    }

    Component {
        id: devModePageComponent
        App.DevelopmentModePage {}
    }
}
