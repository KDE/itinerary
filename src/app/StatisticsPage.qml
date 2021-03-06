/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.itinerary 1.0
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

    Kirigami.FormLayout {
        width: parent.width

        QQC2.ComboBox {
            Kirigami.FormData.isSection: true
            model: timeRangeModel
            textRole: "display"
            onActivated: {
                var range = delegateModel.items.get(currentIndex)
                model.setTimeRange(range.model.begin, range.model.end);
            }
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Total")
        }
        StatisticsDelegate { statItem: model.totalCount }
        StatisticsDelegate { statItem: model.totalDistance }
        StatisticsDelegate { statItem: model.totalNights }
        StatisticsDelegate { statItem: model.totalCO2 }
        StatisticsDelegate { statItem: model.visitedCountries }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Flights")
        }
        StatisticsDelegate { statItem: model.flightCount }
        StatisticsDelegate { statItem: model.flightDistance }
        StatisticsDelegate { statItem: model.flightCO2 }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Trains")
        }
        StatisticsDelegate { statItem: model.trainCount }
        StatisticsDelegate { statItem: model.trainDistance }
        StatisticsDelegate { statItem: model.trainCO2 }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Bus")
        }
        StatisticsDelegate { statItem: model.busCount }
        StatisticsDelegate { statItem: model.busDistance }
        StatisticsDelegate { statItem: model.busCO2 }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Car")
        }
        StatisticsDelegate { statItem: model.carCount }
        StatisticsDelegate { statItem: model.carDistance }
        StatisticsDelegate { statItem: model.carCO2 }
    }
}
