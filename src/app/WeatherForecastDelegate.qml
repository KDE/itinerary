/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.itinerary
import "." as App


FormCard.FormCard {
    id: root
    width: ListView.view.width
    required property var weatherInformation
    readonly property var weatherForecast: weatherInformation.forecast

    visible: weatherForecast.valid
    FormCard.AbstractFormDelegate {
        id: content
        onClicked: if (weatherForecast.range > 1) { applicationWindow().pageStack.push(weatherForecastPage, {weatherInformation: root.weatherInformation}); }
        contentItem: RowLayout {
            spacing: 0

            Kirigami.Icon {
                source: weatherForecast.symbolIconName
                Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                Layout.rightMargin: Kirigami.Units.largeSpacing
            }

            Kirigami.Icon {
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                source: {
                    if (weatherForecast.maximumTemperature > 35.0)
                        return "temperature-warm";
                    if (weatherForecast.minimumTemperature < -20.0)
                        return "temperature-cold";
                    return "temperature-normal"
                }
            }
            QQC2.Label {
                text: weatherForecast.minimumTemperature == weatherForecast.maximumTemperature ?
                    Localizer.formatTemperature(weatherForecast.maximumTemperature) :
                    i18nc("temperature range", "%1 / %2",  Localizer.formatTemperature(weatherForecast.minimumTemperature),
                                                           Localizer.formatTemperature(weatherForecast.maximumTemperature))
                Layout.rightMargin: Kirigami.Units.largeSpacing
            }

            Kirigami.Icon {
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                source: "raindrop"
                color: weatherForecast.precipitation > 25.0 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                visible: weatherForecast.precipitation > 0
            }
            QQC2.Label {
                text: i18nc("precipitation", "%1 mm", weatherForecast.precipitation)
                visible: weatherForecast.precipitation > 0
                Layout.rightMargin: Kirigami.Units.largeSpacing
            }

            Kirigami.Icon {
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                source: "flag"
                color: weatherForecast.windSpeed > 17.5 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                visible: weatherForecast.windSpeed > 3.5
            }
            QQC2.Label {
                text: i18nc("windSpeed", "%1 m/s", weatherForecast.windSpeed)
                visible: weatherForecast.windSpeed > 3.5
            }

            Item { Layout.fillWidth: true }
        }
    }


    Accessible.name: i18n("Weather Forecast")
    Accessible.onPressAction: root.clicked()
}
