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

    WeatherForecastModel {
        id: forecastModel
        weatherForecast: root.weatherInformation.forecast
        weatherForecastManager: WeatherForecastManager
    }

    header: QQC2.Label {
        text: root.weatherInformation.locationName
        padding: Kirigami.Units.largeSpacing
        visible: text
    }

    Component {
        id: weatherForecastDelegate
        Kirigami.AbstractListItem {
            readonly property var fc: model.weatherForecast
            highlighted: false
            backgroundColor: fc.isSevere ? Kirigami.Theme.negativeBackgroundColor : "transparent"

            RowLayout {
                spacing: 0

                Kirigami.Icon {
                    source: weatherForecast.symbolIconName
                    Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                    Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                    Layout.rightMargin: Kirigami.Units.largeSpacing
                }

                QQC2.Label {
                    text: model.localizedTime
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
    }

    ListView {
        anchors.fill: parent
        id: forecastList
        model: forecastModel
        delegate: weatherForecastDelegate
        clip: true

        footer: QQC2.Label {
            width: forecastList.width
            text: i18n("Using data from <a href=\"https://www.met.no/\">The Norwegian Meteorological Institute</a> under <a href=\"https://creativecommons.org/licenses/by/4.0\">Creative Commons 4.0 BY International</a> license.")
            padding: Kirigami.Units.largeSpacing
            font: Kirigami.Theme.smallFont
            wrapMode: Text.WordWrap
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }
}
