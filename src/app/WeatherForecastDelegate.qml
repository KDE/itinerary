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
import org.kde.itinerary.weather

FormCard.FormCard {
    id: root
    width: ListView.view.width
    required property var weatherInformation
    readonly property weatherForecast weatherForecast: weatherInformation.forecast

    visible: weatherForecast.valid
    FormCard.AbstractFormDelegate {
        id: content
        onClicked: if (root.weatherForecast.range > 1) { applicationWindow().pageStack.push(weatherForecastPage, {weatherInformation: root.weatherInformation}); }
        contentItem: RowLayout {
            spacing: 0

            Kirigami.Icon {
                source: root.weatherForecast.symbolIconName
                Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                Layout.rightMargin: Kirigami.Units.largeSpacing
            }

            Kirigami.Icon {
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                source: {
                    if (root.weatherForecast.maximumTemperature > 35.0)
                        return "temperature-warm";
                    if (root.weatherForecast.minimumTemperature < -20.0)
                        return "temperature-cold";
                    return "temperature-normal"
                }
            }
            QQC2.Label {
                text: Localizer.formatTemperatureRange(root.weatherForecast.minimumTemperature, root.weatherForecast.maximumTemperature, Settings.useFahrenheit)
                Layout.rightMargin: Kirigami.Units.largeSpacing
            }

            Kirigami.Icon {
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                source: "raindrop"
                color: root.weatherForecast.precipitation > 25.0 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                visible: root.weatherForecast.precipitation > 0
            }
            QQC2.Label {
                text: i18nc("precipitation", "%1 mm", root.weatherForecast.precipitation)
                visible: root.weatherForecast.precipitation > 0
                Layout.rightMargin: Kirigami.Units.largeSpacing
            }

            Kirigami.Icon {
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                source: "flag"
                color: root.weatherForecast.windSpeed > 17.5 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                visible: root.weatherForecast.windSpeed > 3.5
            }
            QQC2.Label {
                text: i18nc("windSpeed", "%1 m/s", root.weatherForecast.windSpeed)
                visible: root.weatherForecast.windSpeed > 3.5
            }

            Item { Layout.fillWidth: true }
        }
    }


    Accessible.name: i18n("Weather Forecast")
    Accessible.onPressAction: content.clicked()
}
