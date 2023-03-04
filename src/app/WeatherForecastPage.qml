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
            Row {
                spacing: Kirigami.Units.largeSpacing
                Kirigami.Icon {
                    source: fc.symbolIconName
                    isMask: false
                    width: Kirigami.Units.iconSizes.small
                    height: width
                }

                QQC2.Label {
                    text: model.localizedTime
                }

                QQC2.Label {
                    text: {
                        if (fc.maximumTemperature == fc.minimumTemperature) {
                            return i18n("%1°C", fc.maximumTemperature);
                        } else {
                            return i18n("%1°C / %2°C", fc.minimumTemperature, fc.maximumTemperature);
                        }
                    }
                }

                QQC2.Label {
                    visible: fc.precipitation > 0
                    text: i18n("☂ %1 mm", fc.precipitation)
                }

                QQC2.Label {
                    visible: fc.windSpeed > 3.5
                    text: i18n("🌬️ %1 m/s", fc.windSpeed)
                }
            }
        }
    }

    ListView {
        anchors.fill: parent
        id: forecastList
        model: forecastModel
        delegate: weatherForecastDelegate

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
