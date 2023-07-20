/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root

    property var weatherInformation

    title: i18n("Weather Forecast")

    header: QQC2.ToolBar {
        width: root.width

        QQC2.Label {
            text: root.weatherInformation.locationName
            padding: Kirigami.Units.largeSpacing
            visible: text.length > 0
        }
    }

    footer: QQC2.ToolBar {
        width: root.width

        contentItem: Kirigami.Heading {
            level: 2
            text: i18n("Using data from <a href=\"https://www.met.no/\">The Norwegian Meteorological Institute</a> under <a href=\"https://creativecommons.org/licenses/by/4.0\">Creative Commons 4.0 BY International</a> license.")
            font: Kirigami.Theme.smallFont
            wrapMode: Text.WordWrap
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }

    ListView {
        id: forecastList

        model: WeatherForecastModel {
            weatherForecast: root.weatherInformation.forecast
            weatherForecastManager: WeatherForecastManager
        }

        delegate: QQC2.ItemDelegate {
            id: weatherForecastDelegate

            required property var weatherForecast
            required property string localizedTime

            highlighted: false
            width: parent.width

            background: Rectangle {
                color: weatherForecast.isSevere ? Kirigami.Theme.negativeBackgroundColor : Kirigami.Theme.backgroundColor
            }

            contentItem: RowLayout {
                spacing: 0

                Kirigami.Icon {
                    source: weatherForecast.symbolIconName
                    Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                    Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                    Layout.rightMargin: Kirigami.Units.largeSpacing
                }

                QQC2.Label {
                    text: weatherForecastDelegate.localizedTime
                    Layout.rightMargin: Kirigami.Units.largeSpacing
                }

                Kirigami.Icon {
                    Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    Layout.preferredWidth: Kirigami.Units.iconSizes.small
                    source: {
                        if (weatherForecastDelegate.weatherForecast.maximumTemperature > 35.0)
                            return "temperature-warm";
                        if (weatherForecastDelegate.weatherForecast.minimumTemperature < -20.0)
                            return "temperature-cold";
                        return "temperature-normal"
                    }
                }
                QQC2.Label {
                    text: weatherForecastDelegate.weatherForecast.minimumTemperature === weatherForecastDelegate.weatherForecast.maximumTemperature ?
                        Localizer.formatTemperature(weatherForecastDelegate.weatherForecast.maximumTemperature) :
                        i18nc("temperature range", "%1 / %2",  Localizer.formatTemperature(weatherForecastDelegate.weatherForecast.minimumTemperature),
                                                            Localizer.formatTemperature(weatherForecastDelegate.weatherForecast.maximumTemperature))
                    Layout.rightMargin: Kirigami.Units.largeSpacing
                }

                Kirigami.Icon {
                    Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    Layout.preferredWidth: Kirigami.Units.iconSizes.small
                    source: "raindrop"
                    color: weatherForecastDelegate.weatherForecast.precipitation > 25.0 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                    visible: weatherForecastDelegate.weatherForecast.precipitation > 0
                }
                QQC2.Label {
                    text: i18nc("precipitation", "%1 mm", weatherForecastDelegate.weatherForecast.precipitation)
                    visible: weatherForecastDelegate.weatherForecast.precipitation > 0
                    Layout.rightMargin: Kirigami.Units.largeSpacing
                }

                Kirigami.Icon {
                    Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    Layout.preferredWidth: Kirigami.Units.iconSizes.small
                    source: "flag"
                    color: weatherForecastDelegate.weatherForecast.windSpeed > 17.5 ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                    visible: weatherForecastDelegate.weatherForecast.windSpeed > 3.5
                }
                QQC2.Label {
                    text: i18nc("windSpeed", "%1 m/s", weatherForecastDelegate.weatherForecast.windSpeed)
                    visible: weatherForecastDelegate.weatherForecast.windSpeed > 3.5
                }

                Item { Layout.fillWidth: true }
            }
        }
    }
}
