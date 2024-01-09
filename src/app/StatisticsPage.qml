// SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.i18n.localeData
import org.kde.itinerary
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: root
    title: i18n("Statistics")

    property alias reservationManager: model.reservationManager
    property alias tripGroupManager: model.tripGroupManager

    data: [
        StatisticsModel {
            id: model
        },
        StatisticsTimeRangeModel {
            id: timeRangeModel
            reservationManager: model.reservationManager
        }
    ]

    FormCard.FormHeader {}
    FormCard.FormCard {
        FormCard.FormComboBoxDelegate {
            model: timeRangeModel
            text: i18n("Year")
            textRole: "display"
            onActivated: {
                const begin = timeRangeModel.data(timeRangeModel.index(currentIndex, 0), StatisticsTimeRangeModel.BeginRole);
                const end = timeRangeModel.data(timeRangeModel.index(currentIndex, 0), StatisticsTimeRangeModel.EndRole);
                model.setTimeRange(begin, end);
            }
            currentIndex: 0
        }
    }

    FormCard.FormHeader {
        title: i18n("Total")
    }
    FormCard.FormCard {
        StatisticsDelegate { statItem: model.totalCount }
        FormCard.FormDelegateSeparator {}
        StatisticsDelegate { statItem: model.totalDistance }
        FormCard.FormDelegateSeparator {}
        StatisticsDelegate { statItem: model.totalNights }
        FormCard.FormDelegateSeparator {}
        StatisticsDelegate { statItem: model.totalCO2 }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            id: countryDetailsLink
            text: model.visitedCountries.label
            description: model.visitedCountries.value.split(" ").map(countryCode => Country.fromAlpha2(countryCode).emojiFlag).join(" ")
            onClicked: countryDetailsDelegate.visible = !countryDetailsDelegate.visible
            descriptionItem.font.family: 'emoji'
        }

        FormCard.FormDelegateSeparator { visible: countryDetailsDelegate.visible }

        FormCard.AbstractFormDelegate {
            id: countryDetailsDelegate
            background: Item {}
            visible: false
            property var model: visible ? model.visitedCountries.value.split(" ") : []
            contentItem: ColumnLayout {
                Repeater {
                    id: countryDetailsRepeater
                    model: countryDetailsDelegate.model
                    QQC2.Label {
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        readonly property var country: Country.fromAlpha2(modelData)
                        textFormat: Text.RichText
                        text: '<span style="font-family: emoji">' + country.emojiFlag + "</span> " + country.name
                        Accessible.name: country.name
                    }
                }
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Flight")
        visible: model.flightCount.hasData
    }
    FormCard.FormCard {
        visible: model.flightCount.hasData

        StatisticsDelegate { statItem: model.flightCount }
        FormCard.FormDelegateSeparator {}
        StatisticsDelegate { statItem: model.flightDistance }
        FormCard.FormDelegateSeparator {}
        StatisticsDelegate { statItem: model.flightCO2 }
    }

    FormCard.FormHeader {
        title: i18n("Train")
        visible: model.trainCount.hasData
    }
    FormCard.FormCard {
        visible: model.trainCount.hasData

        StatisticsDelegate { statItem: model.trainCount }
        FormCard.FormDelegateSeparator {}
        StatisticsDelegate { statItem: model.trainDistance }
        FormCard.FormDelegateSeparator {}
        StatisticsDelegate { statItem: model.trainCO2 }
    }

    FormCard.FormHeader {
        title: i18n("Bus")
        visible: model.busCount.hasData
    }
    FormCard.FormCard {
        visible: model.busCount.hasData

        StatisticsDelegate { statItem: model.busCount }
        FormCard.FormDelegateSeparator {}
        StatisticsDelegate { statItem: model.busDistance }
        FormCard.FormDelegateSeparator {}
        StatisticsDelegate { statItem: model.busCO2 }
    }

    FormCard.FormHeader {
        title: i18n("Boat")
        visible: model.boatCount.hasData
    }
    FormCard.FormCard {
        visible: model.boatCount.hasData

        StatisticsDelegate { statItem: model.boatCount }
        FormCard.FormDelegateSeparator {}
        StatisticsDelegate { statItem: model.boatDistance }
        FormCard.FormDelegateSeparator {}
        StatisticsDelegate { statItem: model.boatCO2 }
    }

    FormCard.FormHeader {
        title: i18n("Car")
        visible: model.carCount.hasData
    }
    FormCard.FormCard {
        visible: model.carCount.hasData

        StatisticsDelegate { statItem: model.carCount }
        FormCard.FormDelegateSeparator {}
        StatisticsDelegate { statItem: model.carDistance }
        FormCard.FormDelegateSeparator {}
        StatisticsDelegate { statItem: model.carCO2 }
    }
}
