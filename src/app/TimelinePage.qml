/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtCore as QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import Qt.labs.qmlmodels as Models
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components
import org.kde.kpublictransport
import org.kde.itinerary

Kirigami.ScrollablePage {
    id: root
    title: i18n("My Itinerary")
    onBackRequested: event => { event.accepted = true; }
    Kirigami.Theme.colorSet: Kirigami.Theme.Window

    leftPadding: 0
    rightPadding: 0

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

    function addTrainTrip() {
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

        pageStack.clear()
        pageStack.push(Qt.resolvedUrl("JourneyRequestPage.qml"), {
            publicTransportManager: LiveDataManager.publicTransportManager,
            initialCountry: country,
            initialDateTime: dt,
            departureStop: departureLocation
        });
    }

    property list<Kirigami.Action> addActions: [
        Kirigami.Action {
            text: i18n("Add train trip…")
            icon.name: "list-add-symbolic"
            onTriggered: {
                addTrainTrip()
            }
        },
        Kirigami.Action {
            text: i18n("Add flight…")
            icon.name: LineMode.iconName(Line.Air)
            onTriggered: {
                const dt = dateTimeAtIndex(currentIndex());
                let res =  Factory.makeFlightReservation();
                let trip = res.reservationFor;
                trip.departureTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), dt.getHours() == 0 ? 8 : dt.getHours() + 1, 0);
                res.reservationFor = trip;
                applicationWindow().pageStack.push(flightEditorPage, {reservation: res});
            }
        },
        Kirigami.Action {
            text: i18n("Add ferry trip…")
            icon.name: LineMode.iconName(Line.Ferry)
            onTriggered: {
                const dt = dateTimeAtIndex(currentIndex());
                let res =  Factory.makeBoatReservation();
                let trip = res.reservationFor;
                trip.departureTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), dt.getHours() == 0 ? 8 : dt.getHours() + 1, 0);
                res.reservationFor = trip;
                applicationWindow().pageStack.push(boatEditorPage, {reservation: res});
            }
        },
        Kirigami.Action {
            text: i18n("Add accommodation…")
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
            text: i18n("Add event…")
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
            text: i18n("Add restaurant…")
            icon.name: "qrc:///images/foodestablishment.svg"
            onTriggered: {
                const dt = dateTimeAtIndex(currentIndex());
                let res =  Factory.makeFoodEstablishmentReservation();
                res.startTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), 20, 0);
                res.endTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), 22, 0);
                applicationWindow().pageStack.push(restaurantEditorPage, {reservation: res});
            }
        }
    ]
    Components.DoubleFloatingButton {
        id: button
        parent: root.overlay
        anchors {
            right: parent.right
            rightMargin: Kirigami.Settings.isMobile ? Kirigami.Units.largeSpacing : Kirigami.Units.largeSpacing + (root.contentItem.QQC2.ScrollBar && root.contentItem.QQC2.ScrollBar.vertical ? root.contentItem.QQC2.ScrollBar.vertical.width : 0)
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.largeSpacing
        }

        leadingAction: Kirigami.Action {
            text: i18nc("@action:button", "Go To Now")
            icon.name: "view-calendar-day"
            onTriggered: listView.positionViewAtIndex(TripGroupProxyModel.todayRow, ListView.Beginning);
            tooltip: text
        }

        trailingAction: Kirigami.Action{
            text: i18nc("@action:button", "Add trip")
            icon.name: "list-add-symbolic"
            onTriggered: addMenu.open()
            tooltip: text
        }
    }

    SheetDrawer {
        id: addMenu
        headerItem: Kirigami.Heading {
            leftPadding: Kirigami.Units.smallSpacing
            text: i18nc("@title:group", "Add New Trip")
        }
        contentItem: QQC2.Control{
            rightPadding: 0
            leftPadding: 0
            topPadding: Kirigami.Units.smallSpacing * 0.5
            bottomPadding: Kirigami.Units.smallSpacing * 0.5
            contentItem: ColumnLayout {
                Repeater {
                    model: root.addActions
                    delegate: QQC2.ItemDelegate {
                        required property Kirigami.Action modelData
                        Layout.fillWidth: true
                        text: modelData.text
                        icon.name: modelData.icon.name
                        onClicked: {
                            addMenu.close()
                            modelData.triggered()
                        }
                    }
                }
            }
        }
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
        property string suggestedFileName
        title: i18n("Export")
        property list<QQC2.Action> _actions: [
            Kirigami.Action {
                text: i18n("As Itinerary file…")
                icon.name: "export-symbolic"
                onTriggered: {
                    tripGroupFileExportDialog.tripGroupId = exportTripGroupDialog.tripGroupId;
                    tripGroupFileExportDialog.currentFile = exportTripGroupDialog.suggestedFileName + ".itinerary"
                    tripGroupFileExportDialog.open();
                }
            },
            Kirigami.Action {
                text: i18n("As GPX file…")
                icon.name: "map-globe"
                onTriggered: {
                    tripGroupGpxExportDialog.tripGroupId = exportTripGroupDialog.tripGroupId;
                    tripGroupGpxExportDialog.currentFile = exportTripGroupDialog.suggestedFileName + ".gpx"
                    tripGroupGpxExportDialog.open();
                }
            }
        ]
        actions: exportTripGroupDialog._actions
        Instantiator {
            model: KDEConnectDeviceModel {
                id: deviceModel
            }
            delegate: Kirigami.Action {
                text: i18n("Send to %1", model.name)
                icon.name: "kdeconnect-tray"
                onTriggered: ApplicationController.exportTripToKDEConnect(exportTripGroupDialog.tripGroupId, model.deviceId)
            }
            onObjectAdded: exportTripGroupDialog._actions.push(object)
        }
        onVisibleChanged: {
            if (exportTripGroupDialog.visible)
                deviceModel.refresh();
        }
    }
    FileDialog {
        id: tripGroupFileExportDialog
        property string tripGroupId
        fileMode: FileDialog.SaveFile
        title: i18n("Export Trip")
        nameFilters: [i18n("Itinerary file (*.itinerary)")]
        onAccepted: {
            ApplicationController.exportTripToFile(tripGroupFileExportDialog.tripGroupId, tripGroupFileExportDialog.selectedFile);
            Settings.writeFileDialogFolder("tripGroupExport", tripGroupFileExportDialog.selectedFile)
        }
        onVisibleChanged: {
            if (tripGroupFileExportDialog.visible) {
                tripGroupFileExportDialog.currentFolder = Settings.readFileDialogFolder("tripGroupExport",
                    QtCore.StandardPaths.writableLocation(QtCore.StandardPaths.DocumentsLocation));
            }
        }
    }
    FileDialog {
        id: tripGroupGpxExportDialog
        property string tripGroupId
        fileMode: FileDialog.SaveFile
        title: i18n("Export Trip")
        currentFolder: QtCore.StandardPaths.writableLocation(QtCore.StandardPaths.DocumentsLocation)
        nameFilters: [i18n("GPX Files (*.gpx)")]
        onAccepted: {
            ApplicationController.exportTripToGpx(tripGroupGpxExportDialog.tripGroupId, tripGroupGpxExportDialog.selectedFile);
            Settings.writeFileDialogFolder("tripGroupGpxExport", tripGroupGpxExportDialog.selectedFile)
        }
        onVisibleChanged: {
            if (tripGroupGpxExportDialog.visible) {
                tripGroupGpxExportDialog.currentFolder = Settings.readFileDialogFolder("tripGroupGpxExport",
                    QtCore.StandardPaths.writableLocation(QtCore.StandardPaths.DocumentsLocation));
            }
        }
    }
    TripGroupEditorDialog {
        id: tripGroupEditor
        onAccepted: TripGroupManager.updateTripGroup(tripGroupEditor.tripGroupId, tripGroupEditor.tripGroup)
    }
    TripGroupMergeDialog {
        id: tripGroupMergeDialog
    }
    Component {
        id: tripGroupSplitPage
        TripGroupSplitPage {}
    }

    Component {
        id: flightDetailsPage
        FlightPage { editor: flightEditorPage }
    }
    Component {
        id: trainDetailsPage
        TrainPage {}
    }
    Component {
        id: busDetailsPage
        BusPage {}
    }
    Component {
        id: hotelDetailsPage
        HotelPage { editor: hotelEditorPage }
    }
    Component {
        id: eventDetailsPage
        EventPage { editor: eventEditorPage }
    }
    Component {
        id: restaurantDetailsPage
        RestaurantPage { editor: restaurantEditorPage }
    }
    Component {
        id: carRentalDetailsPage
        CarRentalPage {}
    }
    Component {
        id: boatDetailsPage
        BoatPage { editor: boatEditorPage }
    }
    Component {
        id: touristAttractionDetailsPage
        TouristAttractionPage {}
    }
    Component {
        id: weatherForecastPage
        WeatherForecastPage {}
    }

    Component {
        id: flightEditorPage
        FlightEditor {}
    }
    Component {
        id: boatEditorPage
        BoatEditor {}
    }
    Component {
        id: hotelEditorPage
        HotelEditor {}
    }
    Component {
        id: eventEditorPage
        EventEditor {}
    }
    Component {
        id: restaurantEditorPage
        RestaurantEditor {}
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
            FlightDelegate {
                batchId: model.batchId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.Hotel
            HotelDelegate {
                batchId: model.batchId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.TrainTrip
            TrainDelegate {
                batchId: model.batchId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.BusTrip
            BusDelegate {
                batchId: model.batchId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.Restaurant
            RestaurantDelegate {
                batchId: model.batchId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.TouristAttraction
            TouristAttractionDelegate {
                batchId: model.batchId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.Event
            EventDelegate {
                batchId: model.batchId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.CarRental
            CarRentalDelegate {
                batchId: model.batchId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.BoatTrip
            BoatDelegate {
                batchId: model.batchId
                rangeType: model.rangeType
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.TodayMarker
            RowLayout {
                width: ListView.view.width
                Item{ Layout.fillWidth: true }
                QQC2.Label {
                    Layout.maximumWidth: Kirigami.Units.gridUnit * 30
                    Layout.fillWidth: true
                    height: visible ? implicitHeight : 0
                    visible: model.isTodayEmpty
                    text: i18n("Nothing on the itinerary for today.");
                    color: Kirigami.Theme.textColor
                }
                Item{ Layout.fillWidth: true }

            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.LocationInfo
            LocationInfoDelegate {
                locationInfo: model.locationInformation
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.WeatherForecast
            WeatherForecastDelegate {}
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.TripGroup
            TripGroupDelegate {
                onRemoveTrip: (tripGroupId) => {
                    deleteTripGroupWarningDialog.tripGroupId = tripGroupId;
                    deleteTripGroupWarningDialog.open();
                }
            }
        }
        Models.DelegateChoice {
            roleValue: TimelineElement.Transfer
            TransferDelegate {}
        }
    }

    Kirigami.CardsListView {
        id: listView
        model: TripGroupProxyModel
        delegate: chooser
        leftMargin: 0
        rightMargin: 0

        section {
            property: "sectionHeader"
            delegate: TimelineSectionDelegate { day: section }
            criteria: ViewSection.FullString
            labelPositioning: ViewSection.CurrentLabelAtStart | ViewSection.InlineLabels
        }

        footer: Item {
            height: root.width < Kirigami.Units.gridUnit * 30 + button.width * 2 ? button.height : 0
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
        function onEditNewEventReservation(res) {
            applicationWindow().pageStack.push(eventEditorPage, {reservation: res});
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

    footer: null
}
