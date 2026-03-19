/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.i18n.localeData
import org.kde.notification as KNotification
import org.kde.itinerary

FormCard.FormCardPage {
    id: root
    title: i18n("Welcome!")

    FormCard.FormHeader {
        title: i18nc("@title:group", "Setup")
    }

    FormCard.FormCard {
        CountryComboBoxDelegate {
            text: i18n("Home Country")
            model: Country.allCountries.map(c => c.alpha2).sort((lhs, rhs) => {
                return Country.fromAlpha2(lhs).name.localeCompare(Country.fromAlpha2(rhs).name);
            })
            initialCountry: Settings.homeCountryIsoCode
            onActivated: Settings.homeCountryIsoCode = currentValue
            icon.name: "go-home-symbolic"
            description: i18n("Select your home country for currency conversions and power plug compatibility warnings to work.")
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormSwitchDelegate {
            text: i18n("Use online services")
            description: i18n("KDE Itinerary has all features disabled by default that require online access, such as retrieving live traffic data or weather forecasts. You can enable all this here, or check the settings for much more finegrained control over which services to use.")
            icon.name: "globe-symbolic"
            onToggled: {
                Settings.queryLiveData = checked;
                Settings.weatherForecastEnabled = checked;
                Settings.performCurrencyConversion = checked;
                Settings.wikimediaOnlineContentEnabled = checked;
                Settings.autoAddTransfers = checked;
                Settings.autoFillTransfers = checked;

                if (checked) {
                    UnitConversion.syncCurrencyConversionTable();
                }
            }
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormButtonDelegate {
            text: i18n("Settings…")
            description: i18n("Review the settings for more finegrained control over which online services to use.")
            icon.name: "settings-configure"
            onClicked: applicationWindow().pageStack.layers.push(Qt.createComponent("org.kde.itinerary", "SettingsPage"))
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Permissions")
    }
    FormCard.FormCard {
        id: permissionCard
        property bool hasNotificationPermission: KNotification.NotificationPermission.checkPermission()
        FormCard.FormButtonDelegate {
            text: i18n("Request permissions…")
            description: i18n("Additional permissions are required to show notifications.")
            icon.name: "documentinfo"
            icon.color: Kirigami.Theme.neutralTextColor
            visible: !permissionCard.hasNotificationPermission
            function permissionCallback(success) {
                permissionCard.hasNotificationPermission = success;
            }
            onClicked: KNotification.NotificationPermission.requestPermission(permissionCallback)
        }
        FormCard.FormTextDelegate {
            text: i18n("Notification permissions are available")
            description: i18n("No further action required.");
            icon.name: "checkmark"
            icon.color: Kirigami.Theme.positiveTextColor
            visible: permissionCard.hasNotificationPermission
        }
    }

    FormCard.FormHeader {}

    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            text: i18n("Done!")
            icon.name: "dialog-ok"
            onClicked: applicationWindow().pageStack.goBack();
        }
    }
}
