/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.13
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import Qt.labs.platform 1.1
import org.kde.kirigami 2.17 as Kirigami
import org.kde.solidextras 1.0 as Solid
import org.kde.itinerary 1.0
import "." as App

Kirigami.ApplicationWindow {
    title: i18n("KDE Itinerary")
    reachableModeEnabled: false

    width: 480
    height: 720

    FileDialog {
        id: importDialog
        fileMode: FileDialog.OpenFile
        title: i18n("Import Reservation")
        folder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        nameFilters: [i18n("All Files (*.*)"), i18n("PkPass files (*.pkpass)"), i18n("PDF files (*.pdf)"), i18n("KDE Itinerary files (*.itinerary)")]
        onAccepted: ApplicationController.importFromUrl(file)
    }

    FileDialog {
        id: exportDialog
        fileMode: FileDialog.SaveFile
        title: i18n("Export Itinerary Data")
        folder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        nameFilters: [i18n("KDE Itinerary files (*.itinerary)")]
        onAccepted: ApplicationController.exportToFile(file)
    }

    globalDrawer: Kirigami.GlobalDrawer {
        title: i18n("KDE Itinerary")
        titleIcon: "map-symbolic"
        isMenu: true
        actions: [
            Kirigami.Action {
                text: i18n("Import...")
                iconName: "document-open"
                onTriggered: importDialog.open()
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
                enabled: Solid.NetworkStatus.connectivity != Solid.NetworkStatus.No
                onTriggered: {
                    LiveDataManager.checkForUpdates();
                }
            },
            Kirigami.Action {
                text: i18n("Download Maps")
                iconName: "download"
                enabled: Solid.NetworkStatus.connectivity != Solid.NetworkStatus.No
                icon.color: Solid.NetworkStatus.metered != Solid.NetworkStatus.No ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.textColor
                onTriggered: MapDownloadManager.download();
            },
            Kirigami.Action {
                id: statsAction
                text: i18n("Statistics")
                iconName: "view-statistics"
                enabled: pageStack.layers.depth < 2
                onTriggered: pageStack.layers.push(statisticsComponent)
            },
            Kirigami.Action {
                id: healthCertAction
                text: i18n("Health Certificates")
                iconName: "cross-shape"
                onTriggered: {
                    healtCertificateComponent.source = "HealthCertificatePage.qml"
                    pageStack.layers.push(healtCertificateComponent.item)
                }
                enabled: pageStack.layers.depth < 2
                visible: HealthCertificateManager.isAvailable
            },
            Kirigami.Action {
                id: settingsAction
                text: i18n("Settings...")
                iconName: "settings-configure"
                enabled: pageStack.layers.depth < 2
                onTriggered: pageStack.layers.push(settingsComponent)
            },
            Kirigami.Action {
                text: i18n("Export...")
                iconName: "export-symbolic"
                onTriggered: exportDialog.open()
            },
            Kirigami.Action {
                text: i18n("Help")
                iconName: "help-contents"
                enabled: pageStack.layers.depth < 2
                onTriggered: pageStack.layers.push(welcomeComponent)
            },
            Kirigami.Action {
                id: aboutAction
                text: i18n("About")
                iconName: "help-about-symbolic"
                enabled: pageStack.layers.depth < 2
                onTriggered: pageStack.layers.push(aboutComponent)
            },
            Kirigami.Action {
                id: devModeAction
                text: "Development"
                iconName: "tools-report-bug"
                enabled: pageStack.layers.depth < 2
                onTriggered: pageStack.layers.push(devModePageComponent)
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

    Connections {
        target: ApplicationController
        function onInfoMessage(msg) { showPassiveNotification(msg, "short"); }
        function onOpenPageRequested(page) {
            switch (page) {
            case "stats":
                statsAction.trigger();
                break;
            case "healthCert":
                healthCertAction.trigger();
                break;
            default:
                console.warn("Requested to open unknown page", page);
            }
        }
    }

    Connections {
        target: MapDownloadManager
        function onFinished() { showPassiveNotification(i18n("Map download finished."), "short"); }
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
        App.SettingsPage {}
    }
    Component {
        id: aboutComponent
        App.AboutPage {}
    }
    Component {
        id: statisticsComponent
        App.StatisticsPage {
            reservationManager: ReservationManager
            tripGroupManager: TripGroupManager
        }
    }
        // replace loader with component once we depend on KHealthCertificate unconditionally
    Loader {
        id: healtCertificateComponent
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
        id: journeyPathPage
        App.JourneyPathPage {}
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
