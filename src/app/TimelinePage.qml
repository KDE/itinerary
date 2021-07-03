/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import Qt.labs.platform 1.1 as Platform
import org.kde.kirigami 2.17 as Kirigami
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
                onTriggered: listView.positionViewAtIndex(TripGroupProxyModel.todayRow, ListView.Beginning);
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
        id: locationInfoDelegate
        App.LocationInfoDelegate {
            locationInfo: modelData.locationInformation
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
    Component {
        id: transferDelegate
        App.TransferDelegate {
            transfer: modelData.transfer
        }
    }

    Kirigami.OverlaySheet {
        id: deleteTripGroupWarningSheet
        property string tripGroupId

        QQC2.Label {
            text: i18n("Do you really want to delete this trip?")
            wrapMode: Text.WordWrap
        }

        footer: RowLayout {
            QQC2.Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Delete")
                icon.name: "edit-delete"
                onClicked: {
                    deleteTripGroupWarningSheet.sheetOpen = false;
                    TripGroupManager.removeReservationsInGroup(deleteTripGroupWarningSheet.tripGroupId);
                }
            }
        }
    }

    Platform.FileDialog {
        id: tripGroupGpxExportDialog
        property string tripGroupId
        fileMode: Platform.FileDialog.SaveFile
        title: i18n("Export Trip")
        folder: Platform.StandardPaths.writableLocation(Platform.StandardPaths.DocumentsLocation)
        nameFilters: [i18n("GPX Files (*.gpx)")]
        onAccepted: ApplicationController.exportTripToGpx(tripGroupId, file)
    }

    Kirigami.CardsListView {
        id: listView
        model: TripGroupProxyModel

        delegate: Loader {
            property var modelData: model
            height: item ? item.implicitHeight : 0
            width: parent ? parent.width : 0
            sourceComponent: {
                if (!modelData)
                    return;
                switch (modelData.type) {
                    case TimelineElement.Flight: return flightDelegate;
                    case TimelineElement.Hotel: return hotelDelegate;
                    case TimelineElement.TrainTrip: return trainDelegate;
                    case TimelineElement.BusTrip: return busDelegate;
                    case TimelineElement.Restaurant: return restaurantDelegate;
                    case TimelineElement.TouristAttraction: return touristAttractionDelegate;
                    case TimelineElement.Event: return eventDelegate;
                    case TimelineElement.CarRental: return carRentalDelegate;
                    case TimelineElement.TodayMarker: return todayDelegate;
                    case TimelineElement.LocationInfo: return locationInfoDelegate;
                    case TimelineElement.WeatherForecast: return weatherForecastDelegate;
                    case TimelineElement.TripGroup: return tripGrooupDelegate;
                    case TimelineElement.Transfer: return transferDelegate;
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
                x: - 2*Kirigami.Units.largeSpacing
            }
        }
        section.criteria: ViewSection.FullString
        section.labelPositioning: ViewSection.CurrentLabelAtStart | ViewSection.InlineLabels

        Component.onCompleted: {
            if (listView.hasOwnProperty('reuseItems')) { // introduced in Kirigami 5.84 and heavily breaks things for us
                listView.reuseItems = false;
            }
        }
    }

    // work around initial positioning not working correctly below, as at that point
    // listView.height has bogus values. No idea why, possibly delayed layouting in the ScrollablePage,
    // or a side-effect of the binding loop on delegate heights
    Timer {
        id: positionTimer
        interval: 0
        repeat: false
        onTriggered: listView.positionViewAtIndex(TripGroupProxyModel.todayRow, ListView.Beginning);
    }

    Component.onCompleted: positionTimer.start()
}
