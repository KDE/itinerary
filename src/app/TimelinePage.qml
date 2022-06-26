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
import org.kde.kcalendarcore 1.0 as KCalendarCore
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root

    title: i18n("My Itinerary")
    // context drawer content
    actions {
        contextualActions: [
            Kirigami.Action {
                text: i18n("Go To Now")
                iconName: "view-calendar-day"
                onTriggered: listView.positionViewAtIndex(TripGroupProxyModel.todayRow, ListView.Beginning);
            },
            Kirigami.Action {
                text: i18n("Current Ticket")
                iconName: "view-barcode-qr"
                enabled: TimelineModel.currentBatchId !== ""
                onTriggered: showDetailsPageForReservation(TimelineModel.currentBatchId)
            },
            Kirigami.Action {
                text: i18n("Add train trip...")
                iconName: "list-add-symbolic"
                visible: Settings.developmentMode
                onTriggered: {
                    // find date/time at the current screen center
                    var row = -1;
                    for (var i = listView.contentY + listView.height * 0.8; row == -1 && i > listView.contentY; i -= 10) {
                        row = listView.indexAt(0, i);
                    }
                    const idx = listView.model.index(row, 0);

                    const HOUR = 60 * 60 * 1000;
                    var roundInterval = HOUR;
                    var dt;
                    if (listView.model.data(idx, TimelineModel.IsTimeboxedRole) && !listView.model.data(idx, TimelineModel.IsCanceledRole)) {
                        dt = listView.model.data(idx, TimelineModel.EndDateTimeRole);
                        roundInterval = 5 * 60 * 1000;
                    } else {
                        dt = listView.model.data(idx, TimelineModel.StartDateTimeRole);
                    }

                    // clamp to future times and round to the next plausible hour
                    const now = new Date();
                    if (!dt || dt.getTime() < now.getTime()) {
                        dt = now;
                    }
                    if (dt.getTime() % HOUR == 0 && dt.getHours() == 0) {
                        dt.setTime(dt.getTime() + HOUR * 8);
                    } else {
                        dt.setTime(dt.getTime() + roundInterval - (dt.getTime() % roundInterval));
                    }

                    // determine where we are at that time
                    const place = TimelineModel.locationAtTime(dt);
                    var country = Settings.homeCountryIsoCode;
                    var departureLocation;
                    if (place) {
                        country = place.address.addressCountry;
                        departureLocation = PublicTransport.locationFromPlace(place, undefined);
                        departureLocation.name = place.name;
                    }

                    applicationWindow().pageStack.push(Qt.resolvedUrl("JourneyRequestPage.qml"), {
                        publicTransportManager: LiveDataManager.publicTransportManager,
                        initialCountry: country,
                        initialDateTime: dt,
                        departureStop: departureLocation
                    });
                }
            },
            Kirigami.Action {
                iconName: "view-calendar-day"
                text: i18n("Add from calendar...")
                onTriggered: PermissionManager.requestPermission(Permission.ReadCalendar, function() {
                    if (!calendarSelectorListView.model) {
                        calendarSelectorListView.model = calendarModel.createObject(root);
                    }
                    calendarSelector.open();
                })
                visible: KCalendarCore.CalendarPluginLoader.hasPlugin
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

    Component {
        id: calendarModel
        // needs to be created on demand, after we have calendar access permissions
        KCalendarCore.CalendarListModel {}
    }
    Component {
        id: calendarImportPage
        App.CalendarImportPage {}
    }
    Kirigami.OverlaySheet {
        id: calendarSelector
        title: i18n("Select Calendar")

        ListView {
            id: calendarSelectorListView
            delegate: Kirigami.BasicListItem {
                text: model.name
                onClicked: {
                    applicationWindow().pageStack.push(calendarImportPage, {calendar: model.calendar});
                    calendarSelector.close();
                }
            }
        }
    }

    Component {
        id: flightDetailsPage
        App.FlightPage {}
    }
    Component {
        id: trainDetailsPage
        App.TrainPage {}
    }
    Component {
        id: busDetailsPage
        App.BusPage {}
    }
    Component {
        id: hotelDetailsPage
        App.HotelPage {}
    }
    Component {
        id: eventDetailsPage
        App.EventPage {}
    }
    Component {
        id: restaurantDetailsPage
        App.RestaurantPage {}
    }
    Component {
        id: carRentalDetailsPage
        App.CarRentalPage {}
    }
    Component {
        id: touristAttractionDetailsPage
        App.TouristAttractionPage {}
    }

    function detailsComponent(batchId) {
        const res = ReservationManager.reservation(batchId);
        if (!res) {
            return undefined;
        }
        switch (res.className) {
            case "FlightReservation": return flightDetailsPage;
            case "TrainReservation": return trainDetailsPage;
            case "BusReservation": return busDetailsPage;
            case "LodgingReservation": return hotelDetailsPage;
            case "EventReservation": return eventDetailsPage;
            case "FoodEstablishmentReservation": return restaurantDetailsPage;
            case "RentalCarReservation": return carRentalDetailsPage;
            case "TouristAttractionVisit": return touristAttractionDetailsPage;
        }
        console.log("unhandled reservation type:", res.className);
        return undefined;
    }

    function showDetailsPageForReservation(batchId) {
        const c = detailsComponent(batchId);
        if (c) {
            showDetailsPage(c, batchId);
        }
    }

    function showDetailsPage(detailsComponent, batchId) {
        while (applicationWindow().pageStack.depth > 1) {
            applicationWindow().pageStack.pop();
        }
        applicationWindow().pageStack.push(detailsComponent, { batchId: batchId });
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
        section.delegate: TimelineSectionDelegate { day: section }
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
