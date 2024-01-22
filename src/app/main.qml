/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtCore as QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import QtLocation as QtLocation
import org.kde.kirigami as Kirigami
import org.kde.solidextras as Solid
import org.kde.kpublictransport.onboard
import internal.org.kde.kcalendarcore as KCalendarCore
import org.kde.itinerary

Kirigami.ApplicationWindow {
    id: root
    title: i18n("KDE Itinerary")

    width: Kirigami.Settings.isMobile ? 480 : 800
    height: Kirigami.Settings.isMobile ? 720 : 650
    minimumWidth: 300
    minimumHeight: 400
    Kirigami.PagePool {
        id: pagepool
    }
    pageStack {
        columnView.columnResizeMode: Kirigami.ColumnView.SingleColumn

        globalToolBar {
            style: Kirigami.ApplicationHeaderStyle.ToolBar
            showNavigationButtons: Kirigami.ApplicationHeaderStyle.ShowBackButton
        }
    }

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

    property list<Kirigami.Action> importActions: [
        Kirigami.Action {
            text: i18n("Open File...")
            icon.name: "document-open"
            shortcut: StandardKey.Open
            onTriggered: {
                importFileDialog.open();
            }
        },
        Kirigami.Action {
            text: i18n("Paste")
            icon.name: "edit-paste"
            enabled: ApplicationController.hasClipboardContent
            shortcut: StandardKey.Paste
            onTriggered: {
                ApplicationController.importFromClipboard();
            }
        },
        Kirigami.Action {
            text: i18n("Scan Barcode...")
            icon.name: "view-barcode-qr"
            onTriggered: {
                pageStack.layers.push(scanBarcodeComponent);
            }
        },
        Kirigami.Action {
            icon.name: "view-calendar-day"
            text: i18n("Add from Calendar...")
            onTriggered: {
                PermissionManager.requestPermission(Permission.ReadCalendar, function() {
                    if (!calendarSelector.model) {
                        calendarSelector.model = calendarModel.createObject(root);
                    }
                    calendarSelector.open();
                })
            }
            visible: KCalendarCore.CalendarPluginLoader.hasPlugin
        },
        // TODO this should not be hardcoded here, but dynamically filled based on what online ticket
        // sources we support
        Kirigami.Action {
            text: i18n("Deutsche Bahn Online Ticket...")
            icon.name: "download"
            onTriggered: {
                pageStack.layers.push(Qt.createComponent("org.kde.itinerary", "OnlineImportPage"), {
                    source: "db",
                });
            }
        },
        Kirigami.Action {
            text: i18n("SNCF Online Ticket...")
            icon.name: "download"
            onTriggered: {
                pageStack.layers.push(Qt.createComponent("org.kde.itinerary", "OnlineImportPage"), {
                    source: "sncf"
                });
            }
        }
    ]

    FileDialog {
        id: importFileDialog
        fileMode: FileDialog.OpenFile
        title: i18n("Import Reservation")
        currentFolder: QtCore.StandardPaths.writableLocation(QtCore.StandardPaths.DocumentsLocation)
        // Android has no file type selector, we get the superset of all filters there since Qt6 (apart from "all"),
        // so don't set any filters on Android in order to be able to open everything we can read
        nameFilters:  Qt.platform.os === "android" ?
            [i18n("All Files (*.*)")] :
            [i18n("All Files (*.*)"), i18n("PkPass files (*.pkpass)"), i18n("PDF files (*.pdf)"), i18n("iCal events (*.ics)"), i18n("KDE Itinerary files (*.itinerary)")]
        onAccepted: ApplicationController.importFromUrl(selectedFile)
    }

    FileDialog {
        id: exportDialog
        fileMode: FileDialog.SaveFile
        title: i18n("Export Itinerary Data")
        currentFolder: QtCore.StandardPaths.writableLocation(QtCore.StandardPaths.DocumentsLocation)
        nameFilters: [i18n("KDE Itinerary files (*.itinerary)")]
        onAccepted: ApplicationController.exportToFile(selectedFile)
    }

    Component {
        id: calendarModel
        // needs to be created on demand, after we have calendar access permissions
        KCalendarCore.CalendarListModel {}
    }
    Component {
        id: calendarImportPage
        CalendarImportPage {}
    }
    footer: NavigationBar {
        id: navigationbar
    }
    CalendarSelectionSheet {
        id: calendarSelector
        // parent: root.Overlay.overlay
        onCalendarSelected: pageStack.push(calendarImportPage, { calendar: calendar });
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
                icon.name: "document-import"
                children: importActions
            },
            Kirigami.Action {
                text: i18n("Check for Updates")
                icon.name: "view-refresh"
                enabled: Solid.NetworkStatus.connectivity != Solid.NetworkStatus.No
                shortcut: StandardKey.Refresh
                onTriggered: {
                    LiveDataManager.checkForUpdates();
                }
            },
            Kirigami.Action {
                text: i18n("Download Maps")
                icon.name: "download"
                enabled: Solid.NetworkStatus.connectivity != Solid.NetworkStatus.No
                icon.color: Solid.NetworkStatus.metered != Solid.NetworkStatus.No ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.textColor
                onTriggered: MapDownloadManager.download();
            },
            Kirigami.Action {
                id: statsAction
                text: i18n("Statistics")
                icon.name: "view-statistics"
                enabled: pageStack.layers.depth < 2
                onTriggered: pageStack.layers.push(statisticsComponent)
            },
            Kirigami.Action {
                id: healthCertAction
                text: i18n("Health Certificates")
                icon.name: "cross-shape"
                onTriggered: {
                    healtCertificateComponent.source = Qt.resolvedUrl("HealthCertificatePage.qml")
                    pageStack.layers.push(healtCertificateComponent.item)
                }
                enabled: pageStack.layers.depth < 2
                visible: ApplicationController.hasHealthCertificateSupport && ApplicationController.healthCertificateManager.rowCount() > 0
            },
            Kirigami.Action {
                id: liveAction
                text: i18n("Live Status")
                icon.name: "media-playback-playing"
                onTriggered: pageStack.layers.push(Qt.createComponent("org.kde.itinerary", "LiveStatusPage"))
                enabled: pageStack.layers.depth < 2
                visible: onboardStatus.status == OnboardStatus.Onboard
            },
            Kirigami.Action {
                id: settingsAction
                text: i18n("Settings...")
                icon.name: "settings-configure"
                enabled: pageStack.layers.depth < 2
                shortcut: StandardKey.Preferences
                onTriggered: pageStack.layers.push(Qt.createComponent("org.kde.itinerary", "SettingsPage"))
            },
            Kirigami.Action {
                text: i18n("Export...")
                icon.name: "export-symbolic"
                onTriggered: exportDialog.open()
            },
            Kirigami.Action {
                text: i18n("Help")
                icon.name: "help-contents"
                enabled: pageStack.layers.depth < 2
                onTriggered: pageStack.layers.push(Qt.createComponent("org.kde.itinerary", "WelcomePage"))
            },
            Kirigami.Action {
                id: aboutAction
                text: i18n("About")
                icon.name: "help-about-symbolic"
                enabled: pageStack.layers.depth < 2
                onTriggered: pageStack.pushDialogLayer(Qt.resolvedUrl("AboutPage.qml"))
            },
            Kirigami.Action {
                id: devModeAction
                text: "Development"
                icon.name: "tools-report-bug"
                enabled: pageStack.layers.depth < 2
                onTriggered: pageStack.layers.push(Qt.createComponent("org.kde.itinerary", "DevelopmentModePage"));
                visible: Settings.developmentMode
            }
        ]
    }
    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
        actions: {
            if (pageStack.layers.depth > 1)
                return (pageStack.layers.currentItem as Kirigami.Page)?.actions ?? [];
            else (pageStack.currentItem as Kirigami.Page)?.actions ?? [];
        }
    }
    pageStack.initialPage: pagepool.loadPage(Qt.resolvedUrl("TimelinePage.qml"))

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
                pagepool.loadPage("TimelinePage.qml").showDetailsPageForReservation(TimelineModel.currentBatchId);
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
            pageStack.push(Qt.createComponent("org.kde.itinerary", "WelcomePage"));
        }
    }

    Component {
        id: mainPageComponent
        TimelinePage {}
    }
    Component {
        id: scanBarcodeComponent
        BarcodeScannerPage {}
    }
    Component {
        id: statisticsComponent
        StatisticsPage {
            reservationManager: ReservationManager
            tripGroupManager: TripGroupManager
        }
    }
    Component {
        id: passComponent
        PassPage {}
    }
    // replace loader with component once we depend on KHealthCertificate unconditionally
    Loader {
        id: healtCertificateComponent
    }
    Component {
        id: journeySectionPage
        JourneySectionPage {}
    }
    Component {
        id: journeyPathPage
        JourneyPathPage {}
    }
    Component {
        id: indoorMapPage
        IndoorMapPage {}
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

    // workaround for Back key handling on Android causing the application to close
    // on secondary layers if those have no focus (either explicitly or via interaction)
    // ### if there isn't a proepr fix for this, should this happen in Kirigami instead?
    Connections {
        target: pageStack.layers
        function onCurrentItemChanged() {
            pageStack.layers.currentItem.forceActiveFocus();
        }
    }
}
