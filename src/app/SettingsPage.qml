// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtPositioning 5.11
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.i18n.localeData 1.0
import org.kde.kpublictransport.onboard 1.0
import org.kde.itinerary 1.0
import "." as App

FormCard.FormCardPage {
    id: root

    title: i18n("Settings")

    property Component ptBackendPage: Component {
        id: ptBackendPage
        PublicTransportBackendPage {
            publicTransportManager: LiveDataManager.publicTransportManager
        }
    }

    property Component favoriteLocationPage: Component {
        id: favoriteLocationPage
        FavoriteLocationPage {}
    }

    // Home location
    FormCard.FormHeader {
        title: i18n("Home")
    }
    FormCard.FormCard {
        App.CountryComboBoxDelegate {
            text: i18n("Home Country")
            model: Country.allCountries.map(c => c.alpha2).sort((lhs, rhs) => {
                return Country.fromAlpha2(lhs).name.localeCompare(Country.fromAlpha2(rhs).name);
            })
            initialCountry: Settings.homeCountryIsoCode
            onActivated: Settings.homeCountryIsoCode = currentValue
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: i18n("Favorite Locations")
            icon.name: "go-home-symbolic"
            onClicked: applicationWindow().pageStack.layers.push(favoriteLocationPage);
        }
    }

    // Online services
    FormCard.FormHeader {
        title: i18n("Online Services")
    }
    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: i18n("Query Traffic Data")
            checked: Settings.queryLiveData
            onToggled: Settings.queryLiveData = checked
            description: i18n("When enabled, this will query transport provider online services for changes such as delays or gate and platform changes.")
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: i18n("Use insecure services")
            checked: LiveDataManager.publicTransportManager.allowInsecureBackends
            onToggled: LiveDataManager.publicTransportManager.allowInsecureBackends = checked
            description: i18n("Enabling this will also use online services that do not offer transport encryption. This is not recommended, but might be unavoidable when relying on live data from certain providers.")
            descriptionItem.color: Kirigami.Theme.negativeTextColor
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            Kirigami.FormData.isSection: true
            text: i18n("Public Transport Information Sources...")
            icon.name: "settings-configure"
            onClicked: applicationWindow().pageStack.layers.push(ptBackendPage)
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            id: weatherSwitch
            text: i18n("Weather Forecast")
            checked: Settings.weatherForecastEnabled
            onToggled: Settings.weatherForecastEnabled = checked
            description: i18n("Showing weather forecasts will query online services.")
        }

        // ATTENTION do not remove this note, see https://api.met.no/license_data.html
        FormCard.FormTextDelegate {
            description: i18n("Using data from <a href=\"https://www.met.no/\">The Norwegian Meteorological Institute</a> under <a href=\"https://creativecommons.org/licenses/by/4.0\">Creative Commons 4.0 BY International</a> license.")
            visible: weatherSwitch.checked
            onLinkActivated: Qt.openUrlExternally(link)
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            id: autoMapDownload
            text: i18n("Preload Map Data")
            checked: Settings.preloadMapData
            onToggled: Settings.preloadMapData = checked
            description: i18n("Enabling this will download maps for all stations and airports for upcoming trips when connected to Wifi network.")
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            id: currencyConversion
            text: i18n("Currency Conversion")
            checked: Settings.performCurrencyConversion
            onToggled: Settings.performCurrencyConversion = checked
            description: i18n("Enabling this will perform online queries for exchange rates to currencies at travel destinations.")
        }
    }

    // Transfer assistant
    FormCard.FormHeader {
        Kirigami.FormData.isSection: true
        title: i18n("Transfer Assistant")
    }
    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: i18n("Automatically add transfers")
            checked: Settings.autoAddTransfers
            onToggled: Settings.autoAddTransfers = checked
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: i18n("Automatically fill transfers")
            description: i18n("When enabled, this will query transport provider online services automatically for transfer information.")
            checked: Settings.autoFillTransfers
            onToggled: Settings.autoFillTransfers = checked
            enabled: Settings.autoAddTransfers && Settings.queryLiveData
        }
    }


    // Notifications
    FormCard.FormHeader {
        Kirigami.FormData.isSection: true
        title: i18n("Notifications")
    }
    FormCard.FormCard {
        id: notificationCard
        FormCard.FormButtonDelegate {
            text: i18n("Configure Notifications...")
            icon.name: "notifications"
            onClicked: NotificationConfigController.configureNotifications()
            enabled: NotificationConfigController.canConfigureNotification
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: i18n("Show notifications on lock screen")
            checked: Settings.showNotificationOnLockScreen
            onToggled: Settings.showNotificationOnLockScreen = checked
            enabled: NotificationConfigController.canShowOnLockScreen
        }

        FormCard.FormDelegateSeparator {}

        property bool hasNotificationPermission: PermissionManager.checkPermission(Permission.PostNotification)
        FormCard.FormButtonDelegate {
            text: i18n("Request permissions...")
            description: i18n("Additional permissions are required to show notifications.")
            icon.name: "documentinfo"
            icon.color: Kirigami.Theme.neutralTextColor
            visible: !notificationCard.hasNotificationPermission
            onClicked: PermissionManager.requestPermission(Permission.PostNotification, function() {
                notificationCard.hasNotificationPermission = PermissionManager.checkPermission(Permission.PostNotification);
            })
        }
        FormCard.FormTextDelegate {
            text: i18n("Notification permissions are available")
            description: i18n("No further action required.");
            icon.name: "checkmark"
            icon.color: Kirigami.Theme.positiveTextColor
            visible: notificationCard.hasNotificationPermission
        }
    }

    // Wifi access for onboard status information
    property OnboardStatus onboardStatus: OnboardStatus {
        id: onboardStatus
    }
    FormCard.FormHeader {
        Kirigami.FormData.isSection: true
        title: i18n("Onboard Status")
    }
    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            text: i18n("Request permissions...")
            description: i18n("Additional permissions are required to access the Wi-Fi status.")
            icon.name: "documentinfo"
            icon.color: Kirigami.Theme.neutralTextColor
            visible: onboardStatus.status == OnboardStatus.MissingPermissions
            onClicked: onboardStatus.requestPermissions()
        }
        FormCard.FormTextDelegate {
            text: {
                switch (onboardStatus.status) {
                    case OnboardStatus.NotConnected:
                    case OnboardStatus.Onboard:
                        return i18n("Wi-Fi access for onboard information is available.");
                    case OnboardStatus.WifiNotEnabled:
                        return i18n("Wi-Fi is not enabled");
                    case OnboardStatus.LocationServiceNotEnabled:
                        return i18n("Location service is not enabled");
                    case OnboardStatus.NotAvailable:
                        return i18n("Wi-Fi access is not available on this system");
                }
            }
            description: {
                switch (onboardStatus.status) {
                    case OnboardStatus.NotConnected:
                    case OnboardStatus.Onboard:
                        return i18n("No further action required.");
                    case OnboardStatus.WifiNotEnabled:
                        return i18n("Enable Wi-Fi on your system to access onboard information.");
                    case OnboardStatus.LocationServiceNotEnabled:
                        return i18n("Enable the location service on your device to access onboard information.");
                    case OnboardStatus.NotAvailable:
                        return i18n("Onboard information are unfortunately not supported on your device at this time.");
                }
            }
            icon.name: {
                switch (onboardStatus.status) {
                    case OnboardStatus.NotConnected:
                    case OnboardStatus.Onboard:
                        return "checkmark"
                    case OnboardStatus.WifiNotEnabled:
                    case OnboardStatus.LocationServiceNotEnabled:
                        return "documentinfo"
                    case OnboardStatus.NotAvailable:
                        return "dialog-cancel"
                }
            }
            icon.color: {
                switch (onboardStatus.status) {
                    case OnboardStatus.NotConnected:
                    case OnboardStatus.Onboard:
                        return Kirigami.Theme.positiveTextColor
                    case OnboardStatus.WifiNotEnabled:
                    case OnboardStatus.LocationServiceNotEnabled:
                        return Kirigami.Theme.neutralTextColor
                    case OnboardStatus.NotAvailable:
                        return Kirigami.Theme.negativeTextColor
                }
            }
            enabled: onboardStatus.status != OnboardStatus.MissingPermissions
        }
    }

    FormCard.FormHeader {
        title: i18n("Matrix Integration")
        visible: matrixCard.visible
    }
    FormCard.FormCard {
        id: matrixCard
        visible: MatrixController.isAvailable
        FormCard.FormTextDelegate {
            text: MatrixController.manager.infoString.length > 0 ? MatrixController.manager.infoString : MatrixController.manager.connected ? i18n("Logged in as %1", MatrixController.manager.userId) : ""
            visible: text.length > 0
        }
        FormCard.FormButtonDelegate {
            text: i18n("Synchronize rooms")
            icon.name: "view-refresh"
            onClicked: MatrixController.manager.sync()
            visible: MatrixController.manager.connected
        }
        FormCard.FormTextFieldDelegate {
            id: matrixId
            label: i18n("Matrix ID")
            visible: !MatrixController.manager.connected
            text: MatrixController.manager.userId
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormTextFieldDelegate {
            id: matrixPassword
            label: i18n("Password")
            echoMode: TextInput.Password
            visible: !MatrixController.manager.connected
        }
        FormCard.FormDelegateSeparator { above: loginButton }
        FormCard.FormButtonDelegate {
            id: loginButton
            text: MatrixController.manager.connected ? i18n("Log out") : i18n("Log in")
            onClicked: MatrixController.manager.connected ? MatrixController.manager.logout() : MatrixController.manager.login(matrixId.text, matrixPassword.text)
            enabled: MatrixController.manager.connected || (matrixId.text.length > 0 && matrixPassword.text.length > 0)
        }
    }

    FormCard.FormHeader {
        title: i18n("Contributing")
    }
    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: i18n("I contribute to OpenStreetMap")
            description: i18n("Enables OSM editing options.")
            checked: Settings.osmContributorMode
            onToggled: Settings.osmContributorMode = checked
        }
    }
}
