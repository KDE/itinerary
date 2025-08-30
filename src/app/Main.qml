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

        // HACK with KF 6.17 the below no longer shows title bars in mobile mode, use the settings from KTrip instead,
        // which do seem to work
        // globalToolBar {
            // style: Kirigami.Settings.isMobile ? Kirigami.ApplicationHeaderStyle.Breadcrumb : Kirigami.ApplicationHeaderStyle.Auto
            // showNavigationButtons: Kirigami.ApplicationHeaderStyle.ShowBackButton
        // }
        globalToolBar {
            style: Kirigami.ApplicationHeaderStyle.ToolBar
            showNavigationButtons: pageStack.currentIndex > 0 || pageStack.layers.depth > 1 ? Kirigami.ApplicationHeaderStyle.ShowBackButton : 0
        }
    }

    // pop pages when not in use
    Connections {
        target: applicationWindow().pageStack

        function onCurrentIndexChanged(): void {
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

    footer: NavigationBar {
        id: navigationbar
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
                id: liveAction
                text: i18n("Live Status")
                icon.name: "media-playback-playing"
                onTriggered: pageStack.layers.push(Qt.createComponent("org.kde.itinerary", "LiveStatusPage"), { onboardStatus: onboardStatus})
                enabled: pageStack.layers.depth < 2
                visible: onboardStatus.status == OnboardStatus.Onboard
            },
            Kirigami.Action {
                id: settingsAction
                text: i18n("Settings…")
                icon.name: "settings-configure"
                enabled: pageStack.layers.depth < 2
                shortcut: StandardKey.Preferences
                onTriggered: pageStack.layers.push(Qt.createComponent("org.kde.itinerary", "SettingsPage"))
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
    pageStack.initialPage: pagepool.loadPage(Qt.resolvedUrl("TripGroupsPage.qml"))

    DropArea {
        id: topDropArea
        onEntered: {
            if (!drag.hasUrls) {
                drag.accepted = false;
            }
        }
        onDropped: {
            for (var i in drop.urls) {
                ImportController.importFromUrl(drop.urls[i]);
            }
        }
        anchors.fill: parent
    }

    Component {
        id: hotelEditorPage
        HotelEditor {}
    }
    Component {
        id: eventEditorPage
        EventEditor {}
    }
    Component {
        id: restaurantEditorPage
        RestaurantEditor {}
    }

    Connections {
        target: ApplicationController

        function onInfoMessage(msg: string): void {
            showPassiveNotification(msg, "short");
        }

        function onOpenPageRequested(page: string, properties: var): void {
            while (pageStack.layers.depth > 1) {
                pageStack.layers.pop();
            }
            switch (page) {
            case "currentTicket":
                pagepool.loadPage(Qt.resolvedUrl("TripGroupsPage.qml")).openCurrentReservation()
                break;
            case "healthCert":
                root.pageStack.push(Qt.resolvedUrl("HealthCertificatePage.qml"))
                break;
            case "stats":
                root.pageStack.push(Qt.createComponent("org.kde.itinerary", "StatisticsPage"), {
                    reservationManager: ReservationManager,
                    tripGroupManager: TripGroupManager,
                    transferManager: TransferManager,
                });
                break;
            case "live":
                if (onboardStatus.status === OnboardStatus.Onboard) {
                    liveAction.trigger();
                } else {
                    root.showPassiveNotification(i18nc("@info:status", "No onboard information available."));
                }
                break;
            case "journeyRequest":
                properties["publicTransportManager"] = LiveDataManager.publicTransportManager;
                properties["initialCountry"] = Settings.homeCountryIsoCode
                pageStack.push(pagepool.loadPage(Qt.resolvedUrl("JourneyRequestPage.qml")), properties);
                break;
            default:
                console.warn("Requested to open unknown page", page);
            }
        }

        function onEditNewHotelReservation(res: var): void {
            root.pageStack.push(hotelEditorPage, {reservation: res});
        }

        function onEditNewRestaurantReservation(res: var): void {
            root.pageStack.push(restaurantEditorPage, {reservation: res});
        }
        function onEditNewEventReservation(res: var): void {
            root.pageStack.push(eventEditorPage, {reservation: res});
        }
    }

    Connections {
        target: ImportController
        function onShowImportPage(): void {
            if (ImportController.canAutoCommit()) {
                console.log("Auto commit");
                ApplicationController.commitImport(ImportController);
            } else {
                pageStack.layers.push(Qt.createComponent("org.kde.itinerary", "ImportPage"), {
                    controller: ImportController
                });
            }
        }
    }

    Connections {
        target: MapDownloadManager
        function onFinished(): void {
            root.showPassiveNotification(i18n("Map download finished."), "short");
        }
    }

    Component.onCompleted: {
        if (ReservationManager.isEmpty() && TripGroupManager.isEmpty()) {
            pageStack.push(Qt.createComponent("org.kde.itinerary", "WelcomePage"));
        }

        if (Settings.performCurrencyConversion) {
            UnitConversion.syncCurrencyConversionTable();
        }
    }

    Component {
        id: passComponent
        PassPage {}
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
    function osmPlugin(): QtLocation.Plugin {
        if (!__qtLocationOSMPlugin) {
            __qtLocationOSMPlugin = __qtLocationOSMPluginComponent.createObject();
        }
        return __qtLocationOSMPlugin;
    }
    property QtLocation.Plugin __qtLocationOSMPlugin: null
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
    // ### if there isn't a proper fix for this, should this happen in Kirigami instead?
    Connections {
        target: pageStack.layers
        function onCurrentItemChanged(): void {
            root.pageStack.layers.currentItem.forceActiveFocus();
        }
    }

    Connections {
        target: MatrixController.manager.connection
        function onNewKeyVerificationSession(session: var): void {
            root.pageStack.pushDialogLayer(Qt.createComponent("org.kde.itinerary", "MatrixKeyVerificationPage"),
                { session: session },
                { title: i18nc("@title:window", "Matrix Session Verification") }
            );
        }
    }
}
