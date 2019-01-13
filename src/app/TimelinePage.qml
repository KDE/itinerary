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
import org.kde.kirigami 2.4 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root

    title: i18n("My Itinerary")
    // context drawer content
    actions {
        contextualActions: [
            Kirigami.Action {
                text: i18n("Today")
                iconName: "view-calendar-day"
                onTriggered: listView.positionViewAtIndex(_timelineModel.todayRow, ListView.Beginning);
            }
        ]
    }

    // page content
    Component {
        id: flightDelegate
        App.FlightDelegate {
            batchId: modelData.batchId
            rangeType: modelData.rangeType
        }
    }
    Component {
        id: hotelDelegate
        App.HotelDelegate {
            batchId: modelData.batchId
            rangeType: modelData.rangeType
        }
    }
    Component {
        id: trainDelegate
        App.TrainDelegate {
            batchId: modelData.batchId
            rangeType: modelData.rangeType
        }
    }
    Component {
        id: busDelegate
        App.BusDelegate {
            batchId: modelData.batchId
            rangeType: modelData.rangeType
        }
    }
    Component {
        id: restaurantDelegate
        App.RestaurantDelegate {
            batchId: modelData.batchId
            rangeType: modelData.rangeType
        }
    }
    Component {
        id: touristAttractionDelegate
        App.TouristAttractionDelegate {
            batchId: modelData.batchId
            rangeType: modelData.rangeType
        }
    }
    Component {
        id: eventDelegate
        App.EventDelegate {
            batchId: modelData.batchId
            rangeType: modelData.rangeType
        }
    }
    Component {
        id: carRentalDelegate
        App.CarRentalDelegate {
            batchId: modelData.batchId
            rangeType: modelData.rangeType
        }
    }
    Component {
        id: todayDelegate
        Item {
            implicitHeight: visible ? label.implicitHeight : 0
            visible: modelData.isTodayEmpty
            QQC2.Label {
                id: label
                anchors.fill: parent
                text: i18n("Nothing on the itinerary for today.");
                color: Kirigami.Theme.textColor
                horizontalAlignment: Qt.AlignHCenter
            }
        }
    }
    Component {
        id: countryInfoDelegate
        App.CountryInfoDelegate {
            countryInfo: modelData.countryInformation
        }
    }
    Component {
        id: weatherForecastDelegate
        App.WeatherForecastDelegate {
            weatherForecast: modelData.weatherForecast
        }
    }
    Component {
        id: tripGrooupDelegate
        App.TripGroupDelegate {
            tripGroup: modelData.tripGroup
            tripGroupId: modelData.tripGroupId
            rangeType: modelData.rangeType
        }
    }

    Kirigami.CardsListView {
        id: listView
        model: _timelineModel

        delegate: Loader {
            property var modelData: model
            height: item ? item.implicitHeight : 0
            sourceComponent: {
                if (!modelData)
                    return;
                switch (modelData.type) {
                    case TimelineModel.Flight: return flightDelegate;
                    case TimelineModel.Hotel: return hotelDelegate;
                    case TimelineModel.TrainTrip: return trainDelegate;
                    case TimelineModel.BusTrip: return busDelegate;
                    case TimelineModel.Restaurant: return restaurantDelegate;
                    case TimelineModel.TouristAttraction: return touristAttractionDelegate;
                    case TimelineModel.Event: return eventDelegate;
                    case TimelineModel.CarRental: return carRentalDelegate;
                    case TimelineModel.TodayMarker: return todayDelegate;
                    case TimelineModel.CountryInfo: return countryInfoDelegate;
                    case TimelineModel.WeatherForecast: return weatherForecastDelegate;
                    case TimelineModel.TripGroup: return tripGrooupDelegate;
                }
            }
        }

        section.property: "sectionHeader"
        section.delegate: Item {
            implicitHeight: headerItem.implicitHeight + Kirigami.Units.largeSpacing*2
            implicitWidth: ListView.view.width
            Kirigami.BasicListItem {
                id: headerItem
                label: section
                backgroundColor: Kirigami.Theme.backgroundColor
                icon: "view-calendar-day"
            }
        }
        section.criteria: ViewSection.FullString
        section.labelPositioning: ViewSection.CurrentLabelAtStart | ViewSection.InlineLabels
    }

    Component.onCompleted: listView.positionViewAtIndex(_timelineModel.todayRow, ListView.Beginning);
}
