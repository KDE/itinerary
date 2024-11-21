// SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import Qt.labs.qmlmodels as Models
import org.kde.kitemmodels
import org.kde.i18n.localeData
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.itinerary

FormCard.FormCardPage {
    id: root

    title: i18nc("@title", "Profile")

    Component {
        id: favoriteLocationPage
        FavoriteLocationPage {}
    }

    // Home location
    FormCard.FormHeader {
        title: i18n("Home")
    }

    FormCard.FormCard {
        CountryComboBoxDelegate {
            text: i18n("Home Country")
            model: Country.allCountries.map(c => c.alpha2).sort((lhs, rhs) => {
                return Country.fromAlpha2(lhs).name.localeCompare(Country.fromAlpha2(rhs).name);
            })
            initialCountry: Settings.homeCountryIsoCode
            onActivated: Settings.homeCountryIsoCode = currentValue
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: i18n("Favorite Locations")
            icon.name: "go-home-symbolic"
            onClicked: applicationWindow().pageStack.layers.push(favoriteLocationPage);
        }
    }


    FormCard.FormHeader {
        title: i18nc("@title", "Passes and Programs")
    }

    FormCard.FormCard {
        FormCard.AbstractFormDelegate {
            visible: passRepeater.count === 0
            topPadding: Kirigami.Units.gridUnit * 2
            rightPadding: Kirigami.Units.gridUnit * 2
            contentItem: Kirigami.PlaceholderMessage {
                icon.name: "wallet-open"
                text: i18n("No bonus or discount program cards or flat rate passes found.")
            }
        }

        Repeater {
            id: passRepeater

            model: KSortFilterProxyModel {
                sourceModel: PassManager
                filterRowCallback: function(source_row, source_parent) {
                  return sourceModel.data(sourceModel.index(source_row, 0, source_parent), PassManager.StateRole) === "valid";
                };
            }

            delegate: PassDelegate {}
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Manage your Passes")
            onClicked: QQC2.ApplicationWindow.window.pageStack.push(Qt.createComponent("org.kde.itinerary", "PassPage"))
        }
    }

    FormCard.FormHeader {
        title: i18n("Statistics")
    }

    FormCard.FormGridContainer {
        Layout.fillWidth: true

        StatisticsModel {
            id: model
            reservationManager: ReservationManager
            tripGroupManager: TripGroupManager
            transferManager: TransferManager
        }

        StatisticsTimeRangeModel {
            id: timeRangeModel
            reservationManager: ReservationManager

            Component.onCompleted: {
                const begin = timeRangeModel.data(timeRangeModel.index(0, 0), StatisticsTimeRangeModel.BeginRole);
                const end = timeRangeModel.data(timeRangeModel.index(0, 0), StatisticsTimeRangeModel.EndRole);
                model.setTimeRange(begin, end);
            }
        }

        infoCards: [
            FormCard.FormGridContainer.InfoCard {
                title: model.totalCount.label
                subtitle: model.totalCount.value
            },
            FormCard.FormGridContainer.InfoCard {
                title: model.totalDistance.label
                subtitle: model.totalDistance.value
            },
            FormCard.FormGridContainer.InfoCard {
                title: model.totalNights.label
                subtitle: model.totalNights.value
            },
            FormCard.FormGridContainer.InfoCard {
                title: model.totalCO2.label
                subtitle: model.totalCO2.value
            }
        ]
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.smallSpacing

        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "See all Statistics")
            onClicked: QQC2.ApplicationWindow.window.pageStack.push(Qt.createComponent("org.kde.itinerary", "StatisticsPage"), {
                reservationManager: ReservationManager,
                tripGroupManager: TripGroupManager,
                transferManager: TransferManager,
            })
        }
    }
}
