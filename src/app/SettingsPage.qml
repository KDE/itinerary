// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtPositioning 5.11
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.i18n.localeData 1.0
import org.kde.kpublictransport.onboard 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    title: i18n("Settings")
    leftPadding: 0
    rightPadding: 0

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

    ColumnLayout {
        width: parent.width

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                // Home location
                MobileForm.FormCardHeader {
                    title: i18n("Home")
                }

                App.CountryComboBoxDelegate {
                    text: i18n("Home Country")
                    model: Country.allCountries.map(c => c.alpha2).sort((lhs, rhs) => {
                        return Country.fromAlpha2(lhs).name.localeCompare(Country.fromAlpha2(rhs).name);
                    })
                    initialCountry: Settings.homeCountryIsoCode
                    onActivated: Settings.homeCountryIsoCode = currentValue
                }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormButtonDelegate {
                    text: i18n("Favorite Locations")
                    icon.name: "go-home-symbolic"
                    onClicked: applicationWindow().pageStack.layers.push(favoriteLocationPage);
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                // Online services
                MobileForm.FormCardHeader {
                    title: i18n("Online Services")
                }

                MobileForm.FormCheckDelegate {
                    text: i18n("Query Traffic Data")
                    checked: Settings.queryLiveData
                    onToggled: Settings.queryLiveData = checked
                    description: i18n("When enabled, this will query transport provider online services for changes such as delays or gate and platform changes.")
                }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormCheckDelegate {
                    text: i18n("Use insecure services")
                    checked: LiveDataManager.publicTransportManager.allowInsecureBackends
                    onToggled: LiveDataManager.publicTransportManager.allowInsecureBackends = checked
                    description: i18n("Enabling this will also use online services that do not offer transport encryption. This is not recommended, but might be unavoidable when relying on live data from certain providers.")
                    descriptionItem.color: Kirigami.Theme.negativeTextColor
                }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormButtonDelegate {
                    Kirigami.FormData.isSection: true
                    text: i18n("Public Transport Information Sources...")
                    icon.name: "settings-configure"
                    onClicked: applicationWindow().pageStack.layers.push(ptBackendPage)
                }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormCheckDelegate {
                    id: weatherSwitch
                    text: i18n("Weather Forecast")
                    checked: Settings.weatherForecastEnabled
                    onToggled: Settings.weatherForecastEnabled = checked
                    description: i18n("Showing weather forecasts will query online services.")
                }

                // ATTENTION do not remove this note, see https://api.met.no/license_data.html
                MobileForm.FormTextDelegate {
                    description: i18n("Using data from <a href=\"https://www.met.no/\">The Norwegian Meteorological Institute</a> under <a href=\"https://creativecommons.org/licenses/by/4.0\">Creative Commons 4.0 BY International</a> license.")
                    visible: weatherSwitch.checked
                    onLinkActivated: Qt.openUrlExternally(link)
                }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormCheckDelegate {
                    id: autoMapDownload
                    text: i18n("Preload Map Data")
                    checked: Settings.preloadMapData
                    onToggled: Settings.preloadMapData = checked
                    description: i18n("Enabling this will download maps for all stations and airports for upcoming trips when connected to Wifi network.")
                }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormCheckDelegate {
                    id: currencyConversion
                    text: i18n("Currency Conversion")
                    checked: Settings.performCurrencyConversion
                    onToggled: Settings.performCurrencyConversion = checked
                    visible: Settings.hasCurrencyConversion
                    description: i18n("Enabling this will perform online queries for exchange rates to currencies at travel destinations.")
                }
            }
        }


        // Transfer assistant
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    Kirigami.FormData.isSection: true
                    title: i18n("Transfer Assistant")
                }

                MobileForm.FormCheckDelegate {
                    text: i18n("Automatically add transfers")
                    checked: Settings.autoAddTransfers
                    onToggled: Settings.autoAddTransfers = checked
                }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormCheckDelegate {
                    text: i18n("Automatically fill transfers")
                    description: i18n("When enabled, this will query transport provider online services automatically for transfer information.")
                    checked: Settings.autoFillTransfers
                    onToggled: Settings.autoFillTransfers = checked
                    enabled: Settings.autoAddTransfers && Settings.queryLiveData
                }
            }
        }


        // Notifications
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    Kirigami.FormData.isSection: true
                    title: i18n("Notifications")
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Configure Notifications...")
                    icon.name: "notifications"
                    onClicked: NotificationConfigController.configureNotifications()
                    enabled: NotificationConfigController.canConfigureNotification
                }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormCheckDelegate {
                    text: i18n("Show notifications on lock screen")
                    checked: Settings.showNotificationOnLockScreen
                    onToggled: Settings.showNotificationOnLockScreen = checked
                    enabled: NotificationConfigController.canShowOnLockScreen
                }
            }
        }

        // Wifi access for onboard status information
        OnboardStatus {
            id: onboardStatus
        }
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    Kirigami.FormData.isSection: true
                    title: i18n("Onboard Status")
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Request permissions...")
                    description: i18n("Additional permissions are required to access the Wi-Fi status.")
                    icon.name: "documentinfo"
                    icon.color: Kirigami.Theme.neutralTextColor
                    visible: onboardStatus.status == OnboardStatus.MissingPermissions
                    onClicked: onboardStatus.requestPermissions()
                }

                MobileForm.FormTextDelegate {
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
        }
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    Kirigami.FormData.isSection: true
                    title: i18n("Matrix Integration")
                    subtitle: MatrixManager.infoString.length > 0 ? MatrixManager.infoString : MatrixManager.connected ? i18n("Logged in as %1", MatrixManager.userId) : ""
                }
                MobileForm.FormButtonDelegate {
                    text: i18n("Synchronize rooms")
                    icon.name: "view-refresh"
                    onClicked: MatrixManager.sync()
                    enabled: MatrixManager.connected
                }
                MobileForm.FormTextFieldDelegate {
                    id: matrixId
                    label: i18n("Matrix ID")
                    enabled: !MatrixManager.connected
                }
                MobileForm.FormTextFieldDelegate {
                    id: matrixPassword
                    label: i18n("Password")
                    echoMode: TextInput.Password
                    enabled: !MatrixManager.connected
                }
                MobileForm.FormButtonDelegate {
                    text: MatrixManager.connected ? i18n("Log out") : i18n("Log in")
                    icon.name: "go-next"
                    onClicked: MatrixManager.connected ? MatrixManager.logout() : MatrixManager.login(matrixId.text, matrixPassword.text)
                    enabled: MatrixManager.connected || (matrixId.text.length > 0 && matrixPassword.text.length > 0)
                }
                Repeater {
                    model: MatrixRoomsModel {
                        connection: MatrixManager.connection
                    }
                    delegate: MobileForm.FormButtonDelegate {
                        text: model.displayName
                        icon.name: "go-next"
                        onClicked: MatrixManager.postLocation(model.id, 52.52745, 13.32466, "KDAB office")
                    }
                }
            }
        }
    }
}
