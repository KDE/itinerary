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

Kirigami.AbstractCard {
    id: root
    property var weatherForecast
    visible: weatherForecast.valid
    headerOrientation: Qt.Horizontal
    showClickFeedback: weatherForecast.range > 1

    header: Rectangle {
        id: headerBackground
        Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
        implicitWidth: icon.implicitWidth + Kirigami.Units.largeSpacing * 2
        Layout.minimumHeight: implicitWidth
        Layout.fillHeight: true
        anchors.leftMargin: -root.leftPadding
        anchors.topMargin: -root.topPadding
        anchors.bottomMargin: -root.rightPadding

        Kirigami.Icon {
            id: icon
            anchors.fill: parent
            anchors.margins: Kirigami.Units.largeSpacing
            source: weatherForecast.symbolIconName
        }
    }

    contentItem: ColumnLayout {
        Layout.fillWidth: true
        QQC2.Label {
            text: i18n("Temperature: %1 °C / %2 °C", weatherForecast.minimumTemperature, weatherForecast.maximumTemperature)
            color: Kirigami.Theme.textColor
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: i18n("Precipitation: %1 mm", weatherForecast.precipitation)
            color: Kirigami.Theme.textColor
            Layout.fillWidth: true
        }
    }

    Component {
        id: detailsComponent
        App.WeatherForecastPage {
            weatherForecast: root.weatherForecast
        }
    }

    onClicked: if (weatherForecast.range > 1) { applicationWindow().pageStack.push(detailsComponent); }
}
