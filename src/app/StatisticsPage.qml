// SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.i18n.localeData 1.0
import org.kde.itinerary 1.0
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import "." as App

Kirigami.ScrollablePage {
    id: root
    title: i18n("Statistics")

    property alias reservationManager: model.reservationManager
    property alias tripGroupManager: model.tripGroupManager

    StatisticsModel {
        id: model
    }

    StatisticsTimeRangeModel {
        id: timeRangeModel
        reservationManager: model.reservationManager
    }

    leftPadding: 0
    rightPadding: 0

    ColumnLayout {
        width: parent.width
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormComboBoxDelegate {
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
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Total")
                }

                StatisticsDelegate { statItem: model.totalCount }
                MobileForm.FormDelegateSeparator {}
                StatisticsDelegate { statItem: model.totalDistance }
                MobileForm.FormDelegateSeparator {}
                StatisticsDelegate { statItem: model.totalNights }
                MobileForm.FormDelegateSeparator {}
                StatisticsDelegate { statItem: model.totalCO2 }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormButtonDelegate {
                    id: countryDetailsLink
                    text: model.visitedCountries.label
                    description: model.visitedCountries.value.split(" ").map(countryCode => Country.fromAlpha2(countryCode).emojiFlag).join(" ")
                    onClicked: countryDetailsDelegate.visible = !countryDetailsDelegate.visible
                    descriptionItem.font.family: 'emoji'
                }

                MobileForm.FormDelegateSeparator { visible: countryDetailsDelegate.visible }

                MobileForm.AbstractFormDelegate {
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
                            }
                        }
                    }
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            visible: model.flightCount.hasData

            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Flight")
                }

                StatisticsDelegate { statItem: model.flightCount }
                MobileForm.FormDelegateSeparator {}
                StatisticsDelegate { statItem: model.flightDistance }
                MobileForm.FormDelegateSeparator {}
                StatisticsDelegate { statItem: model.flightCO2 }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            visible: model.trainCount.hasData

            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Train")
                }
                StatisticsDelegate { statItem: model.trainCount }
                MobileForm.FormDelegateSeparator {}
                StatisticsDelegate { statItem: model.trainDistance }
                MobileForm.FormDelegateSeparator {}
                StatisticsDelegate { statItem: model.trainCO2 }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            visible: model.busCount.hasData

            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Bus")
                }
                StatisticsDelegate { statItem: model.busCount }
                MobileForm.FormDelegateSeparator {}
                StatisticsDelegate { statItem: model.busDistance }
                MobileForm.FormDelegateSeparator {}
                StatisticsDelegate { statItem: model.busCO2 }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            visible: model.boatCount.hasData

            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Boat")
                }
                StatisticsDelegate { statItem: model.boatCount }
                MobileForm.FormDelegateSeparator {}
                StatisticsDelegate { statItem: model.boatDistance }
                MobileForm.FormDelegateSeparator {}
                StatisticsDelegate { statItem: model.boatCO2 }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            visible: model.carCount.hasData

            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Car")
                }
                StatisticsDelegate { statItem: model.carCount }
                MobileForm.FormDelegateSeparator {}
                StatisticsDelegate { statItem: model.carDistance }
                MobileForm.FormDelegateSeparator {}
                StatisticsDelegate { statItem: model.carCO2 }
            }
        }
    }
}
