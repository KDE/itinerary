/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import Qt.labs.qmlmodels 1.0 as Models
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

    Models.DelegateChooser {
        id: chooser
        role: "type"
        Models.DelegateChoice {
            roleValue: TimelineElement.Flight
            App.FlightDelegate {
                batchId: model.batchId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.Hotel
            App.HotelDelegate {
                batchId: model.batchId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.TrainTrip
            App.TrainDelegate {
                batchId: model.batchId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.BusTrip
            App.BusDelegate {
                batchId: model.batchId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.Restaurant
            App.RestaurantDelegate {
                batchId: model.batchId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.TouristAttraction
            App.TouristAttractionDelegate {
                batchId: model.batchId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.Event
            App.EventDelegate {
                batchId: model.batchId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.CarRental
            App.CarRentalDelegate {
                batchId: model.batchId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.TodayMarker
            QQC2.Label {
                height: visible ? implicitHeight : 0
                visible: model.isTodayEmpty
                text: i18n("Nothing on the itinerary for today.");
                color: Kirigami.Theme.textColor
                horizontalAlignment: Qt.AlignHCenter
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.LocationInfo
            App.LocationInfoDelegate {
                locationInfo: model.locationInformation
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.WeatherForecast
            App.WeatherForecastDelegate {
                weatherForecast: model.weatherForecast
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.TripGroup
            App.TripGroupDelegate {
                tripGroup: model.tripGroup
                tripGroupId: model.tripGroupId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.Transfer
            App.TransferDelegate {
                transfer: model.transfer
            }
        }
    }

    Kirigami.CardsListView {
        id: listView
        model: TripGroupProxyModel
        delegate: chooser

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
