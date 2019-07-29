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
import org.kde.kirigami 2.0 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

Kirigami.Page {
    id: root
    property alias weatherForecast: forecastModel.weatherForecast
    title: i18n("Weather Forecast")

    WeatherForecastModel {
        id: forecastModel
        weatherForecastManager: _weatherForecastManager
    }

    ListView {
        anchors.fill: parent
        id: forecastList
        model: forecastModel
        delegate: Kirigami.BasicListItem {
            icon: model.weatherForecast.symbolIconName
            iconColor: "transparent"
            label: {
                var fc = model.weatherForecast;
                if (fc.maximumTemperature == fc.minimumTemperature) {
                    if (fc.precipitation == 0)
                        return i18n("%1 %2°C", model.localizedTime, fc.maximumTemperature);
                    else
                        return i18n("%1 %2°C ☂ %3mm", model.localizedTime, fc.maximumTemperature, fc.precipitation);
                }
                if (fc.precipitation == 0)
                    return i18n("%1 %2°C / %3°C", model.localizedTime, fc.minimumTemperature, fc.maximumTemperature);
                return i18n("%1 %2°C / %3°C ☂ %4 mm", model.localizedTime, model.weatherForecast.minimumTemperature, model.weatherForecast.maximumTemperature, model.weatherForecast.precipitation);
            }
        }
    }
}
