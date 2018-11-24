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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.4 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    title: i18n("Settings")

    CountryModel {
        id: countryModel
    }

    GridLayout {
        columns: 2
        width: root.width

        QQC2.Label {
            text: i18n("Home Country")
        }
        QQC2.ComboBox {
            model: countryModel
            textRole: "display"
            currentIndex: countryModel.isoCodeToIndex(_settings.homeCountryIsoCode)
            onActivated: _settings.homeCountryIsoCode = countryModel.isoCodeFromIndex(currentIndex)
        }

        QQC2.Label {
            text: i18n("Weather Forecast")
        }
        QQC2.Switch {
            id: weatherSwitch
            checked: _settings.weatherForecastEnabled
            onToggled: _settings.weatherForecastEnabled = checked
        }
        QQC2.Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: i18n("Showing weather forecasts will query online services.")
            visible: !weatherSwitch.checked
        }
        // ATTENTION do not remove this note, see https://api.met.no/license_data.html
        QQC2.Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: i18n("Using data from <a href=\"https://www.met.no/\">The Norwegian Meteorological Institute</a> under <a href=\"https://creativecommons.org/licenses/by/4.0\">Creative Commons 4.0 BY International</a> license.")
            visible: weatherSwitch.checked
            wrapMode: Text.WordWrap
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }

    onBackRequested: pageStack.pop()
}
