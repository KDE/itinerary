/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.13
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import Qt.labs.platform 1.1
import QtLocation 5.11 as QtLocation
import org.kde.kirigami 2.17 as Kirigami
import org.kde.solidextras 1.0 as Solid
import org.kde.kpublictransport.onboard 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ApplicationWindow {
    title: i18n("KDE Itinerary")
    reachableModeEnabled: false

    width: Kirigami.Settings.isMobile ? 480 : 800
    height: Kirigami.Settings.isMobile ? 720 : 650
    minimumWidth: 300
    minimumHeight: 400

    pageStack.columnView.columnResizeMode: Kirigami.ColumnView.SingleColumn

    // pop pages when not in use
    Connections {
        target: applicationWindow().pageStack
        function onCurrentIndexChanged() {
            // wait for animation to finish before popping pages
            timer.restart();
        }
    }
    
    Timer {
        id: timer
        interval: 300
        onTriggered: {
            let currentIndex = applicationWindow().pageStack.currentIndex;
            while (applicationWindow().pageStack.depth > (currentIndex + 1) && currentIndex >= 0) {
                applicationWindow().pageStack.pop();
            }
        }
    }


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

    OnboardStatus {
        id: onboardStatus
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
                text: i18n("Scan Barcode...")
                iconName: "view-barcode-qr"
                onTriggered: pageStack.layers.push(scanBarcodeComponent)
            },
            Kirigami.Action {
                text: i18n("Paste")
                iconName: "edit-paste"
                onTriggered: ApplicationController.importFromClipboard()
                enabled: ApplicationController.hasClipboardContent
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
                id: passAction
                text: Kirigami.Settings.isMobile ? i18n("Passes && Programs") : i18n("Passes & Programs") // TODO Kirigami or style bug?
                iconName: "wallet-open"
                onTriggered: pageStack.push(passComponent)
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
                id: liveAction
                text: i18n("Live Status")
                iconName: "media-playback-playing"
                onTriggered: pageStack.push(liveStatusPage)
                enabled: pageStack.layers.depth < 2
                visible: onboardStatus.status == OnboardStatus.Onboard
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
            while (pageStack.layers.depth > 1) {
                pageStack.layers.pop();
            }
            switch (page) {
            case "currentTicket":
                applicationWindow().pageStack.get(0, false).showDetailsPageForReservation(TimelineModel.currentBatchId);
                break;
            case "stats":
                statsAction.trigger();
                break;
            case "healthCert":
                healthCertAction.trigger();
                break;
            case "live":
                liveAction.trigger();
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
        id: scanBarcodeComponent
        App.BarcodeScannerPage {}
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
    Component {
        id: programMembershipPage
        App.ProgramMembershipPage {}
    }
    Component {
        id: passComponent
        App.PassPage {}
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
        id: liveStatusPage
        App.LiveStatusPage {}
    }

    Component {
        id: devModePageComponent
        App.DevelopmentModePage {}
    }

    // "singleton" OSM QtLocation plugin
    // we only want one of these, and created only when absolutely necessary
    // as this triggers network operations on creation already
    function osmPlugin() {
        if (!__qtLocationOSMPlugin) {
            __qtLocationOSMPlugin = __qtLocationOSMPluginComponent.createObject();
        }
        return __qtLocationOSMPlugin;
    }
    property var __qtLocationOSMPlugin: null
    Component {
        id: __qtLocationOSMPluginComponent
        QtLocation.Plugin {
            name: "osm"
            QtLocation.PluginParameter { name: "osm.useragent"; value: ApplicationController.userAgent }
            QtLocation.PluginParameter { name: "osm.mapping.providersrepository.address"; value: "https://autoconfig.kde.org/qtlocation/" }
        }
    }
}
