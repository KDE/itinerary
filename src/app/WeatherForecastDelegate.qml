/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
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

    contentItem: RowLayout {
        spacing: 0

        anchors {
            fill: parent
            leftMargin: Kirigami.Units.largeSpacing
            topMargin: -Kirigami.Units.largeSpacing
        }

        Kirigami.Icon {
            source: weatherForecast.symbolIconName
            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
            Layout.preferredWidth: Kirigami.Units.iconSizes.medium
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
        Component.onCompleted: {
            // HACK: the Material style has negativeBackgroundColor == negativeTextColor, which makes content unreadable
            if (Qt.platform.os !== 'android') {
                root.background.defaultColor = Qt.binding(function() { return weatherForecast.isSevere ? Kirigami.Theme.negativeBackgroundColor : Kirigami.Theme.backgroundColor; });
            }
        }
    }

    onClicked: if (weatherForecast.range > 1) { applicationWindow().pageStack.push(weatherForecastPage, {weatherInformation: root.weatherInformation}); }

    Accessible.name: i18n("Weather Forecast")
    Accessible.onPressAction: root.clicked()
}
