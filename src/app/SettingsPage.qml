/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import QtPositioning 5.11
import org.kde.kirigami 2.4 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    title: i18n("Settings")

    Component {
        id: ptBackendPage
        PublicTransportBackendPage {
            publicTransportManager: LiveDataManager.publicTransportManager
        }
    }

    Component {
        id: favoriteLocationPage
        FavoriteLocationPage {}
    }

    CountryModel {
        id: countryModel
    }

    Kirigami.FormLayout {
        width: root.width

        // Home location
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Home")
        }

        QQC2.ComboBox {
            Kirigami.FormData.label: i18n("Home Country")
            model: countryModel
            textRole: "display"
            currentIndex: countryModel.isoCodeToIndex(Settings.homeCountryIsoCode)
            onActivated: Settings.homeCountryIsoCode = countryModel.isoCodeFromIndex(currentIndex)
        }

        QQC2.Button {
            Kirigami.FormData.isSection: true
            text: i18n("Favorite Locations")
            icon.name: "go-home-symbolic"
            onClicked: applicationWindow().pageStack.push(favoriteLocationPage);
        }

        // Online services
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Online Services")
        }

        QQC2.Switch {
            Kirigami.FormData.label: i18n("Query Traffic Data")
            checked: Settings.queryLiveData
            onToggled: Settings.queryLiveData = checked
        }
        QQC2.Label {
            Kirigami.FormData.isSection: true
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: i18n("When enabled, this will query transport provider online services for changes such as delays or gate and platform changes.")
        }
        QQC2.Switch {
            Kirigami.FormData.label: i18n("Use insecure services")
            checked: LiveDataManager.publicTransportManager.allowInsecureBackends
            onToggled: LiveDataManager.publicTransportManager.allowInsecureBackends = checked
        }
        QQC2.Label {
            Kirigami.FormData.isSection: true
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: i18n("Enabling this will also use online services that do not offer transport encryption. This is not recommended, but might be unavoidable when relying on live data from certain providers.")
            color: Kirigami.Theme.negativeTextColor
        }
        QQC2.Button {
            Kirigami.FormData.isSection: true
            text: i18n("Public Transport Information Sources...")
            icon.name: "settings-configure"
            onClicked: applicationWindow().pageStack.push(ptBackendPage)
        }

        QQC2.Switch {
            id: weatherSwitch
            Kirigami.FormData.label: i18n("Weather Forecast")
            checked: Settings.weatherForecastEnabled
            onToggled: Settings.weatherForecastEnabled = checked
        }
        QQC2.Label {
            Kirigami.FormData.isSection: true
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: i18n("Showing weather forecasts will query online services.")
            visible: !weatherSwitch.checked
        }
        // ATTENTION do not remove this note, see https://api.met.no/license_data.html
        QQC2.Label {
            Kirigami.FormData.isSection:true
            Layout.fillWidth: true
            text: i18n("Using data from <a href=\"https://www.met.no/\">The Norwegian Meteorological Institute</a> under <a href=\"https://creativecommons.org/licenses/by/4.0\">Creative Commons 4.0 BY International</a> license.")
            visible: weatherSwitch.checked
            wrapMode: Text.WordWrap
            onLinkActivated: Qt.openUrlExternally(link)
        }

        QQC2.Switch {
            id: autoMapDownload
            Kirigami.FormData.label: i18n("Preload Map Data")
            checked: Settings.preloadMapData
            onToggled: Settings.preloadMapData = checked
        }
        QQC2.Label {
            Kirigami.FormData.isSection: true
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: i18n("Enabling this will download maps for all stations and airports for upcoming trips when connected to Wifi network.")
        }

        // Transfer assistant
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Transfer Assistant")
        }

        QQC2.Switch {
            Kirigami.FormData.label: i18n("Automatically add transfers")
            checked: Settings.autoAddTransfers
            onToggled: Settings.autoAddTransfers = checked
        }
        QQC2.Switch {
            Kirigami.FormData.label: i18n("Automatically fill transfers")
            checked: Settings.autoFillTransfers
            onToggled: Settings.autoFillTransfers = checked
            enabled: Settings.autoAddTransfers && Settings.queryLiveData
        }
        QQC2.Label {
            Kirigami.FormData.isSection: true
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: i18n("When enabled, this will query transport provider online services automatically for transfer information.")
            enabled: Settings.autoAddTransfers && Settings.queryLiveData
        }

        // Notifications
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Notifications")
        }

        QQC2.Button {
            Kirigami.FormData.isSection: true
            text: i18n("Configure Notifications...")
            icon.name: "notifications"
            onClicked: NotificationConfigController.configureNotifications()
            enabled: NotificationConfigController.canConfigureNotification
        }
        QQC2.Switch {
            Kirigami.FormData.label: i18n("Show notifications on lock screen")
            checked: Settings.showNotificationOnLockScreen
            onToggled: Settings.showNotificationOnLockScreen = checked
            enabled: NotificationConfigController.canShowOnLockScreen
        }
    }
}
