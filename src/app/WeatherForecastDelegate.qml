/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.AbstractCard {
    id: root

    required property var weatherInformation
    readonly property var weatherForecast: weatherInformation.forecast

    visible: weatherForecast.valid
    headerOrientation: Qt.Horizontal
    showClickFeedback: weatherForecast.range > 1

    header: Rectangle {
        id: headerBackground
        Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
        Kirigami.Theme.inherit: false
        color: weatherForecast.isSevere ? Kirigami.Theme.negativeBackgroundColor : Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.smallSpacing
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
            text: weatherForecast.minimumTemperature == weatherForecast.maximumTemperature ?
                i18n("Temperature: %1°C", weatherForecast.maximumTemperature) :
                i18n("Temperature: %1°C / %2°C", weatherForecast.minimumTemperature, weatherForecast.maximumTemperature)
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
            weatherInformation: root.weatherInformation
        }
    }

    onClicked: if (weatherForecast.range > 1) { applicationWindow().pageStack.push(detailsComponent); }

    Accessible.name: i18n("Weather Forecast")
    Accessible.onPressAction: root.clicked()
}
