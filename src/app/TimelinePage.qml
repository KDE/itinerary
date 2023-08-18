/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import Qt.labs.qmlmodels 1.0 as Models
import Qt.labs.platform 1.1 as Platform
import org.kde.kirigami 2.19 as Kirigami
import org.kde.itinerary 1.0
import "." as App

Kirigami.ScrollablePage {
    id: root

    title: i18n("My Itinerary")
    onBackRequested: event => { event.accepted = true; }

    /** Model index somewhat at the center of the currently display timeline. */
    function currentIndex() {
        let row = -1;
        for (let i = listView.contentY + listView.height * 0.8; row == -1 && i > listView.contentY; i -= 10) {
            row = listView.indexAt(0, i);
        }
        return listView.model.index(row, 0);
    }

    /** Date/time the given model index refers to, depending on the type of the element this refers to. */
    function dateTimeAtIndex(idx) {
        if (listView.model.data(idx, TimelineModel.IsTimeboxedRole) && !listView.model.data(idx, TimelineModel.IsCanceledRole)) {
            return listView.model.data(idx, TimelineModel.EndDateTimeRole);
        }
        return listView.model.data(idx, TimelineModel.StartDateTimeRole);
    }

    // context drawer content
    actions {
        contextualActions: [
            Kirigami.Action {
                text: i18n("Go To Now")
                icon.name: "view-calendar-day"
                onTriggered: listView.positionViewAtIndex(TripGroupProxyModel.todayRow, ListView.Beginning);
            },
            Kirigami.Action {
                text: i18n("Current Ticket")
                icon.name: "view-barcode-qr"
                enabled: TimelineModel.currentBatchId !== ""
                onTriggered: showDetailsPageForReservation(TimelineModel.currentBatchId)
            },
            Kirigami.Action {
                text: i18n("Add train trip...")
                icon.name: "list-add-symbolic"
                onTriggered: {
                    // find date/time at the current screen center
                    const idx = currentIndex();

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
                text: i18n("Add accommodation...")
                icon.name: "go-home-symbolic"
                onTriggered: {
                    const dt = dateTimeAtIndex(currentIndex());
                    let res =  Factory.makeLodgingReservation();
                    res.checkinTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), 15, 0);
                    res.checkoutTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate() + 1, 11, 0);
                    applicationWindow().pageStack.push(hotelEditorPage, {reservation: res});
                }
            },
            Kirigami.Action {
                text: i18n("Add event...")
                icon.name: "meeting-attending"
                onTriggered: {
                    const dt = dateTimeAtIndex(currentIndex());
                    let res = Factory.makeEventReservation();
                    let ev = res.reservationFor;
                    ev.startDate = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), dt.getHours() == 0 ? 8 : dt.getHours() + 1, 0);
                    res.reservationFor = ev;
                    applicationWindow().pageStack.push(eventEditorPage, {reservation: res});
                }
            },
            Kirigami.Action {
                text: i18n("Add restaurant...")
                icon.source: "qrc:///images/foodestablishment.svg"
                onTriggered: {
                    const dt = dateTimeAtIndex(currentIndex());
                    let res =  Factory.makeFoodEstablishmentReservation();
                    res.startTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), 20, 0);
                    res.endTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), 22, 0);
                    applicationWindow().pageStack.push(restaurantEditorPage, {reservation: res});
                }
            }
        ]
    }

    // page content
    Kirigami.PromptDialog {
        id: deleteTripGroupWarningDialog
        property string tripGroupId

        title: i18n("Delete Trip")
        subtitle: i18n("Do you really want to delete this trip?")

        standardButtons: QQC2.Dialog.Cancel

        customFooterActions: [
            Kirigami.Action {
                text: i18n("Delete")
                icon.name: "edit-delete"
                onTriggered: {
                    TripGroupManager.removeReservationsInGroup(deleteTripGroupWarningDialog.tripGroupId);
                    deleteTripGroupWarningDialog.close();
                }
            }
        ]
    }

    Kirigami.MenuDialog {
        id: exportTripGroupDialog
        property string tripGroupId
        title: i18n("Export")
        actions: [
            Kirigami.Action {
                text: i18n("As Itinerary file...")
                icon.name: "export-symbolic"
                onTriggered: {
                    exportTripGroupDialog.close();
                    tripGroupFileExportDialog.tripGroupId = exportTripGroupDialog.tripGroupId;
                    tripGroupFileExportDialog.open();
                }
            },
            Kirigami.Action {
                text: i18n("As GPX file...")
                icon.name: "map-globe"
                onTriggered: {
                    exportTripGroupDialog.close();
                    tripGroupGpxExportDialog.tripGroupId = exportTripGroupDialog.tripGroupId;
                    tripGroupGpxExportDialog.open();
                }
            }
        ]
    }
    Platform.FileDialog {
        id: tripGroupFileExportDialog
        property string tripGroupId
        fileMode: Platform.FileDialog.SaveFile
        title: i18n("Export Trip")
        folder: Platform.StandardPaths.writableLocation(Platform.StandardPaths.DocumentsLocation)
        nameFilters: [i18n("Itinerary file (*.itinerary)")]
        onAccepted: ApplicationController.exportTripToFile(tripGroupId, file)
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
        id: boatDetailsPage
        App.BoatPage {}
    }
    Component {
        id: touristAttractionDetailsPage
        App.TouristAttractionPage {}
    }
    Component {
        id: weatherForecastPage
        App.WeatherForecastPage {}
    }

    Component {
        id: hotelEditorPage
        App.HotelEditor {}
    }
    Component {
        id: eventEditorPage
        App.EventEditor {}
    }
    Component {
        id: restaurantEditorPage
        App.RestaurantEditor {}
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
            roleValue: TimelineElement.BoatTrip
            App.BoatDelegate {
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
            App.WeatherForecastDelegate {}
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.TripGroup
            App.TripGroupDelegate {
                onRemoveTrip: (tripGroupId) => {
                    deleteTripGroupWarningDialog.tripGroupId = tripGroupId;
                    deleteTripGroupWarningDialog.open();
                }
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.Transfer
            App.TransferDelegate {}
        }
    }

    Kirigami.CardsListView {
        id: listView
        model: TripGroupProxyModel
        delegate: chooser

        section {
            property: "sectionHeader"
            delegate: TimelineSectionDelegate { day: section }
            criteria: ViewSection.FullString
            labelPositioning: ViewSection.CurrentLabelAtStart | ViewSection.InlineLabels
        }
    }

    Connections {
        target: ApplicationController
        function onEditNewHotelReservation(res) {
            applicationWindow().pageStack.push(hotelEditorPage, {reservation: res});
        }
        function onEditNewRestaurantReservation(res) {
            applicationWindow().pageStack.push(restaurantEditorPage, {reservation: res});
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
