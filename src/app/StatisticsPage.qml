/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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
import org.kde.kirigami 2.4 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root
    title: i18n("Statistics")
    property alias reservationManager: model.reservationManager

    StatisticsModel {
        id: model
    }

    StatisticsTimeRangeModel {
        id: timeRangeModel
        reservationManager: model.reservationManager
    }

    Kirigami.FormLayout {

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
