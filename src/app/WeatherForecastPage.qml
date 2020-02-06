/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.Page {
    id: root
    property alias weatherForecast: forecastModel.weatherForecast
    title: i18n("Weather Forecast")

    WeatherForecastModel {
        id: forecastModel
        weatherForecastManager: WeatherForecastManager
    }

    Component {
        id: weatherForecastDelegate
        Kirigami.AbstractListItem {
            readonly property var fc: model.weatherForecast
            highlighted: false
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
    }
}
