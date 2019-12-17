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
import QtPositioning 5.11
import org.kde.kirigami 2.4 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    title: i18n("Settings")

    Component {
        id: ptBackendPage
        PublicTransportBackendPage {
            publicTransportManager: _liveDataManager.publicTransportManager
        }
    }

    Component {
        id: locationPickerPage
        LocationPicker {
            title: i18n("Pick Home Location")
            coordinate: QtPositioning.coordinate(TransferManager.homeLatitude, TransferManager.homeLongitude)
            onCoordinateChanged: {
                TransferManager.homeLatitude = coordinate.latitude;
                TransferManager.homeLongitude = coordinate.longitude;
            }
        }
    }

    CountryModel {
        id: countryModel
    }

    Kirigami.FormLayout {
        width: root.width

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Home")
        }

        QQC2.ComboBox {
            Kirigami.FormData.label: i18n("Home Country")
            model: countryModel
            textRole: "display"
            currentIndex: countryModel.isoCodeToIndex(_settings.homeCountryIsoCode)
            onActivated: _settings.homeCountryIsoCode = countryModel.isoCodeFromIndex(currentIndex)
        }

        QQC2.Button {
            Kirigami.FormData.isSection: true
            text: i18n("Pick Home Location")
            icon.name: "crosshairs"
            onClicked: applicationWindow().pageStack.push(locationPickerPage);
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Online Services")
        }

        QQC2.Switch {
            Kirigami.FormData.label: i18n("Query Traffic Data")
            checked: _settings.queryLiveData
            onToggled: _settings.queryLiveData = checked
        }
        QQC2.Label {
            Kirigami.FormData.isSection: true
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: i18n("When enabled, this will query transport provider online services for changes such as delays or gate and platform changes.")
        }
        QQC2.Switch {
            Kirigami.FormData.label: i18n("Use insecure services")
            checked: _liveDataManager.publicTransportManager.allowInsecureBackends
            onToggled: _liveDataManager.publicTransportManager.allowInsecureBackends = checked
        }
        QQC2.Label {
            Kirigami.FormData.isSection: true
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: i18n("Enabling this will also use online services that do not offer transport encryption. This is not recommended, but might be unavoidable when relying on live data from certain providers.")
            color: Kirigami.Theme.negativeTextColor
        }
        QQC2.Button {
            Kirigami.FormData.isSection: true
            text: i18n("Public Transport Information Sources...")
            icon.name: "settings-configure"
            onClicked: applicationWindow().pageStack.push(ptBackendPage)
        }

        QQC2.Switch {
            id: weatherSwitch
            Kirigami.FormData.label: i18n("Weather Forecast")
            checked: _settings.weatherForecastEnabled
            onToggled: _settings.weatherForecastEnabled = checked
        }
        QQC2.Label {
            Kirigami.FormData.isSection: true
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: i18n("Showing weather forecasts will query online services.")
            visible: !weatherSwitch.checked
        }
        // ATTENTION do not remove this note, see https://api.met.no/license_data.html
        QQC2.Label {
            Kirigami.FormData.isSection:true
            Layout.fillWidth: true
            text: i18n("Using data from <a href=\"https://www.met.no/\">The Norwegian Meteorological Institute</a> under <a href=\"https://creativecommons.org/licenses/by/4.0\">Creative Commons 4.0 BY International</a> license.")
            visible: weatherSwitch.checked
            wrapMode: Text.WordWrap
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }
}
