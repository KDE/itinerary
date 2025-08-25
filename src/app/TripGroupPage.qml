// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtCore as QtCore
import QtNetwork as QtNetwork
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import Qt.labs.qmlmodels
import QtQml.Models
import org.kde.coreaddons as CoreAddons
import org.kde.i18n.localeData
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components as Components
import org.kde.kpublictransport as KPublicTransport
import org.kde.itinerary
import org.kde.itinerary.weather

Kirigami.ScrollablePage {
    id: root

    required property var tripGroup
    required property string tripGroupId

    readonly property bool isEmptyTripGroup: isNaN(root.tripGroup.beginDateTime.getTime())

    property TripGroupController controller: TripGroupController {
        id: _controller
        tripGroupModel: TripGroupModel
        tripGroupId: root.tripGroupId
        weatherForecastManager: WeatherForecastManager
        homeCountryIsoCode: Settings.homeCountryIsoCode
        homeCurrency: Country.fromAlpha2(Settings.homeCountryIsoCode).currencyCode
        transferManager: TransferManager
        convertCurrency: Settings.performCurrencyConversion
    }

    title: tripGroup.name

    Kirigami.Theme.colorSet: Kirigami.Theme.Window

    Connections {
        target: TripGroupManager
        function onTripGroupChanged(tgId : string) {
            if (tgId === root.tripGroupId)
                root.tripGroup = TripGroupManager.tripGroup(root.tripGroupId);
        }
    }

    /** Model index somewhat at the center of the currently display timeline. */
    function currentIndex() {
        let row = -1;
        for (let i = listView.contentY + listView.height * 0.8; row == -1 && i > listView.contentY; i -= 10) {
            row = listView.indexAt(0, i);
        }
        if (row === -1) {
            row = listView.count - 1;
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

    function openEmptyReservationEditor(reservationType: int): void {
        const dt = root.isEmptyTripGroup ? new Date() : root.dateTimeAtIndex(root.currentIndex());
        let res;
        let editorPage;
        switch (reservationType) {
        case KPublicTransport.Line.Air:
            res = Factory.makeFlightReservation();
            editorPage = Qt.createComponent("org.kde.itinerary", "FlightEditor")
            break;
        case KPublicTransport.Line.Train:
            res = Factory.makeTrainReservation();
            editorPage = Qt.createComponent("org.kde.itinerary", "TrainEditor")
            break;
        case KPublicTransport.Line.Bus:
            res = Factory.makeBusReservation();
            editorPage = Qt.createComponent("org.kde.itinerary", "BusEditor")
            break;
        case KPublicTransport.Line.Ferry:
            res = Factory.makeBoatReservation();
            editorPage = Qt.createComponent("org.kde.itinerary", "BoatEditor")
        }
        let trip = res.reservationFor;
        if (dt) {
            trip.departureTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), dt.getHours() == 0 ? 8 : dt.getHours() + 1, 0);
        }
        res.reservationFor = trip;
        applicationWindow().pageStack.push(editorPage, {reservation: res});
    }

    function searchConnection(): void {
        const HOUR = 60 * 60 * 1000;
        let roundInterval = HOUR;
        let dt = new Date();

        if (!root.isEmptyTripGroup) {
            // find date/time at the current screen center
            const idx = currentIndex();

            if (listView.model.data(idx, TimelineModel.IsTimeboxedRole) && !listView.model.data(idx, TimelineModel.IsCanceledRole)) {
                dt = listView.model.data(idx, TimelineModel.EndDateTimeRole);
                roundInterval = 5 * 60 * 1000;
            } else {
                dt = listView.model.data(idx, TimelineModel.StartDateTimeRole);
            }
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
        const place = TripGroupModel.locationAtTime(dt);
        var country = Settings.homeCountryIsoCode;
        var departureLocation;
        if (place) {
            country = place.address.addressCountry;
            departureLocation = PublicTransport.locationFromPlace(place);
        }

        pageStack.clear()
        pageStack.push(Qt.resolvedUrl("JourneyRequestPage.qml"), {
            publicTransportManager: LiveDataManager.publicTransportManager,
            initialCountry: country,
            initialDateTime: dt,
            departureStop: departureLocation
        });
    }

    property list<QQC2.Action> addActions: [
        ImportAction {
            id: importAction

            pageStack: applicationWindow().pageStack
        },
        QQC2.Action {
            id: searchConnectionAction

            text: i18nc("@action", "Search connection")
            icon.name: "search-symbolic"
            onTriggered: {
                searchConnection()
            }
        },
        Kirigami.Action {
            text: i18nc("@action", "Add connection")
            icon.name: KPublicTransport.LineMode.iconName(KPublicTransport.Line.Train)

            QQC2.Action {
                id: addTrainAction

                text: i18nc("@action", "Add train")
                icon.name: KPublicTransport.LineMode.iconName(KPublicTransport.Line.Train)
                onTriggered: {
                    root.openEmptyReservationEditor(KPublicTransport.Line.Train);
                }
            }

            QQC2.Action {
                id: addBusAction

                text: i18nc("@action", "Add bus")
                icon.name: KPublicTransport.LineMode.iconName(KPublicTransport.Line.Bus)
                onTriggered: {
                    root.openEmptyReservationEditor(KPublicTransport.Line.Bus);
                }
            }

            QQC2.Action {
                id: addFlightAction

                text: i18nc("@action", "Add flight")
                icon.name: KPublicTransport.LineMode.iconName(KPublicTransport.Line.Air)
                onTriggered: {
                    root.openEmptyReservationEditor(KPublicTransport.Line.Air);
                }
            }

            QQC2.Action {
                id: addFerryAction

                text: i18nc("@action", "Add ferry trip")
                icon.name: KPublicTransport.LineMode.iconName(KPublicTransport.Line.Ferry)
                onTriggered: {
                    root.openEmptyReservationEditor(KPublicTransport.Line.Ferry);
                }
            }
        },
        QQC2.Action {
            id: addAccommodationAction

            text: i18nc("@action", "Add accommodation")
            icon.name: "go-home-symbolic"
            onTriggered: {
                const dt = root.isEmptyTripGroup ? new Date() : root.dateTimeAtIndex(root.currentIndex());
                let res = Factory.makeLodgingReservation();
                if (dt) {
                    res.checkinTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), 15, 0);
                    res.checkoutTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate() + 1, 11, 0);
                }
                applicationWindow().pageStack.push(hotelEditorPage, {reservation: res});
            }
        },
        QQC2.Action {
            id: addEventAction

            text: i18nc("@action", "Add event")
            icon.name: "meeting-attending"
            onTriggered: {
                const dt = root.isEmptyTripGroup ? new Date() : root.dateTimeAtIndex(root.currentIndex());
                let res = Factory.makeEventReservation();
                let ev = res.reservationFor;
                if (dt) {
                    ev.startDate = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), dt.getHours() == 0 ? 8 : dt.getHours() + 1, 0);
                }
                res.reservationFor = ev;
                applicationWindow().pageStack.push(eventEditorPage, {reservation: res});
            }
        },
        QQC2.Action {
            id: addRestaurantAction

            text: i18n("Add restaurant")
            icon.name: KPublicTransport.FeatureType.typeIconName(KPublicTransport.Feature.Restaurant)
            onTriggered: {
                const dt = root.isEmptyTripGroup ? new Date() : root.dateTimeAtIndex(root.currentIndex());
                let res =  Factory.makeFoodEstablishmentReservation();
                if (dt) {
                    res.startTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), 20, 0);
                    res.endTime = new Date(dt.getFullYear(), dt.getMonth(), dt.getDate(), 22, 0);
                }
                applicationWindow().pageStack.push(restaurantEditorPage, {reservation: res});
            }
        }
    ]

    Components.ConvergentContextMenu {
        id: addMenu
        headerContentItem: Kirigami.Heading {
            text: i18nc("@title:group", "Add to Trip")
        }

        Component.onCompleted: if (!Kirigami.Settings.isMobile) {
            displayMode = Components.ConvergentContextMenu.Dialog;
        }
        parent: root.QQC2.Overlay.overlay

        actions: root.addActions
    }

    TripGroupEditorDialog {
        id: tripGroupEditor
        onAccepted: {
            TripGroupManager.updateTripGroup(tripGroupEditor.tripGroupId, tripGroupEditor.tripGroup);
            root.tripGroup = TripGroupManager.tripGroup(root.tripGroupId);
        }
    }

    TripGroupMergeDialog {
        id: tripGroupMergeDialog
        onAccepted: root.tripGroup = TripGroupManager.tripGroup(root.tripGroupId)
    }

    Component {
        id: tripGroupSplitPage
        TripGroupSplitPage {
            onSplitDone: root.tripGroup = TripGroupManager.tripGroup(root.tripGroupId)
        }
    }

    ExportMenuDialog {
        id: exportTripGroupDialog
        title: i18n("Export Trip")
        settingsKey: "tripGroup"
        suggestedName: root.tripGroup.slugName
        onExportToFile: (path) => { ApplicationController.exportTripToFile(root.tripGroupId, path); }
        onExportToGpxFile: (path) => { ApplicationController.exportTripToGpx(root.tripGroupId, path); }
        onExportToKDEConnect: (deviceId) => { ApplicationController.exportTripToKDEConnect(root.tripGroupId, deviceId); }
    }

    Kirigami.PromptDialog {
        id: deleteTripGroupWarningDialog

        title: i18n("Delete Trip")
        subtitle: i18n("Do you really want to delete the trip '%1'?", root.tripGroup.name)

        standardButtons: QQC2.Dialog.Cancel

        customFooterActions: [
            Kirigami.Action {
                text: i18n("Delete")
                icon.name: "edit-delete"
                onTriggered: {
                    TripGroupManager.removeReservationsInGroup(root.tripGroupId);
                    deleteTripGroupWarningDialog.close();
                    applicationWindow().pageStack.pop();
                }
            }
        ]
    }

    Component {
        id: tripGroupMapPage
        TripGroupMapPage { tripGroupId: root.tripGroupId }
    }

    Component {
        id: flightDetailsPage
        FlightPage {}
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
        BoatPage {}
    }
    Component {
        id: touristAttractionDetailsPage
        TouristAttractionPage {}
    }
    Component {
        id: weatherForecastPage
        WeatherForecastPage {}
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
        while (applicationWindow().pageStack.depth > 2) {
            applicationWindow().pageStack.pop();
        }
        applicationWindow().pageStack.push(detailsComponent, { batchId: batchId });
    }

    TimelineModel {
        id: timelineModel
        homeCountryIsoCode: Settings.homeCountryIsoCode
        reservationManager: ReservationManager
        weatherForecastManager: WeatherForecastManager
        transferManager: TransferManager
        tripGroupManager: TripGroupManager
        tripGroupId: root.tripGroupId
        initialLocation: TripGroupModel.locationAtTime(root.tripGroup.beginDateTime)
    }

    ListView {
        id: listView
        topMargin: Kirigami.Units.gridUnit
        spacing: Kirigami.Units.largeSpacing

        header: ColumnLayout {
            width: listView.width
            spacing: 0

            FormCard.FormCard {
                Layout.bottomMargin: listView.spacing
                visible: !root.isEmptyTripGroup

                FormCard.FormTextDelegate {
                    id: dateLabel
                    readonly property int dayCount: Math.ceil((root.tripGroup.endDateTime.getTime() - root.tripGroup.beginDateTime.getTime()) / (1000 * 3600 * 24))
                    text: dateLabel.dayCount <= 1 ? Localizer.formatDate(root.tripGroup, "beginDateTime")
                        : i18n("%1 - %2",  Localizer.formatDate(root.tripGroup, "beginDateTime"),  Localizer.formatDate(root.tripGroup, "endDateTime"))
                    description: dateLabel.dayCount >= 1 ? i18np("One day", "%1 days", dateLabel.dayCount) : ""
                }

                FormCard.FormDelegateSeparator {}

                FormCard.FormTextDelegate {
                    visible: root.controller.weatherForecast.valid
                    icon.name:  root.controller.weatherForecast.symbolIconName
                    text: Localizer.formatTemperatureRange(root.controller.weatherForecast.minimumTemperature, root.controller.weatherForecast.maximumTemperature, Settings.useFahrenheit)
                }

                FormCard.FormDelegateSeparator {
                    visible: root.controller.weatherForecast.valid
                }

                Repeater {
                    model: root.controller.locationInformation

                    ColumnLayout {
                        spacing: 0

                        FormCard.FormDelegateSeparator {
                            visible: index > 0 || root.controller.weatherForecast.valid
                        }

                        FormCard.FormTextDelegate {
                            icon.name: "documentinfo"
                            icon.color: modelData.powerPlugCompatibility == LocationInformation.PartiallyCompatible ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.negativeTextColor
                            text: Country.fromAlpha2(modelData.isoCode).name
                            description: {
                                if (modelData.powerPlugCompatibility == LocationInformation.PartiallyCompatible) {
                                    if (modelData.powerPlugTypes.length === 0) {
                                        return i18n("Some incompatible power sockets (%1)", modelData.powerSocketTypes);
                                    } else {
                                        return i18n("Some incompatible power plugs (%1)", modelData.powerPlugTypes);
                                    }
                                }
                                return i18n("No compatible power plugs (%1)", modelData.powerSocketTypes);
                            }
                        }
                    }
                }

                FormCard.FormDelegateSeparator {
                    visible: root.controller.locationInformation.length > 0
                }

                FormCard.FormTextDelegate {
                    icon.name: "view-currency-list"
                    text: root.controller.currencies.length > 0 ? i18np("Currency: %2", "Currencies: %2", root.controller.currencies.length, root.controller.currencies.join(", ")) : ""
                    visible: root.controller.currencies.length > 0
                }

                FormCard.FormDelegateSeparator {
                    visible: root.controller.currencies.length > 0
                }

                FormCard.FormButtonDelegate {
                    icon.name: "map-globe"
                    text: i18n("Show map…")
                    visible: !root.isEmptyTripGroup
                    onClicked: applicationWindow().pageStack.push(tripGroupMapPage);
                }
            }

            FormCard.FormHeader {
                title: i18n("Import")
                visible: root.isEmptyTripGroup
            }
            FormCard.FormCard {
                visible: root.isEmptyTripGroup

                Repeater {
                    model: importAction.children
                    FormCard.FormButtonDelegate {
                        required property Kirigami.Action modelData
                        action: modelData
                        visible: modelData.visible
                    }
                }
            }

            FormCard.FormHeader {
                title: i18n("Add")
                visible: root.isEmptyTripGroup
            }
            FormCard.FormCard {
                visible: root.isEmptyTripGroup
                Repeater {
                    model: [
                        searchConnectionAction,
                        addTrainAction,
                        addBusAction,
                        addFlightAction,
                        addFerryAction,
                        addEventAction,
                        addAccommodationAction,
                        addRestaurantAction
                    ]

                    FormCard.FormButtonDelegate {
                        required property QQC2.Action modelData
                        action: modelData
                    }
                }
            }
        }

        model: root.isEmptyTripGroup ? null : timelineModel

        section {
            property: "sectionHeader"
            delegate: TimelineSectionDelegate { day: section }
            criteria: ViewSection.FullString
        }

        delegate: DelegateChooser {
            role: "type"
            DelegateChoice {
                roleValue: TimelineElement.Flight
                FlightDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: flightDetailsPage
                }
            }
            DelegateChoice {
                roleValue: TimelineElement.Hotel
                HotelDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: hotelDetailsPage
                }
            }
            DelegateChoice {
                roleValue: TimelineElement.TrainTrip
                TrainDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: trainDetailsPage
                }
            }
            DelegateChoice {
                roleValue: TimelineElement.BusTrip
                BusDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: busDetailsPage
                }
            }
            DelegateChoice {
                roleValue: TimelineElement.Restaurant
                RestaurantDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: restaurantDetailsPage
                }
            }
            DelegateChoice {
                roleValue: TimelineElement.TouristAttraction
                TouristAttractionDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: touristAttractionDetailsPage
                }
            }
            DelegateChoice {
                roleValue: TimelineElement.Event
                EventDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: eventDetailsPage
                }
            }
            DelegateChoice {
                roleValue: TimelineElement.CarRental
                CarRentalDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: carRentalDetailsPage
                }
            }
            DelegateChoice {
                roleValue: TimelineElement.BoatTrip
                BoatDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: boatDetailsPage
                }
            }
            DelegateChoice {
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
            DelegateChoice {
                roleValue: TimelineElement.LocationInfo
                LocationInfoDelegate {
                    locationInfo: model.locationInformation
                }
            }
            DelegateChoice {
                roleValue: TimelineElement.WeatherForecast
                WeatherForecastDelegate {}
            }
            DelegateChoice {
                roleValue: TimelineElement.Transfer
                TransferDelegate {}
            }
        }

        footer: ColumnLayout {
            width: listView.width
            spacing: 0

            FormCard.FormHeader {
                title: i18n("Statistics")
                visible: distanceStats.visible || co2Stats.visible || costStats.visible
            }
            FormCard.FormCard {
                visible: distanceStats.visible || co2Stats.visible || costStats.visible
                FormCard.FormTextDelegate {
                    id: distanceStats
                    text: i18n("Distance")
                    description: CoreAddons.Format.formatDistance(root.controller.totalDistance, Settings.distanceFormat)
                    visible: root.controller.totalDistance > 0
                }
                FormCard.FormDelegateSeparator {
                    visible: distanceStats.visible
                }
                FormCard.FormTextDelegate {
                    id: co2Stats
                    text: i18n("CO₂")
                    description: Localizer.formatWeight(root.controller.totalCO2Emission)
                    visible: root.controller.totalCO2Emission > 0
                }
                FormCard.FormDelegateSeparator {
                    visible: co2Stats.visible
                }
                FormCard.FormTextDelegate {
                    id: costStats
                    text: i18n("Cost")
                    description: Localizer.formatCurrency(root.controller.totalCost.value, root.controller.totalCost.currency)
                    visible: !isNaN(root.controller.totalCost.value)
                }
            }

            FormCard.FormHeader {
                title: i18n("Actions")
            }

            FormCard.FormCard {
                FormCard.FormButtonDelegate {
                    action: Kirigami.Action {
                        text: i18n("Check for Updates")
                        icon.name: "view-refresh"
                        enabled: QtNetwork.NetworkInformation.reachability === QtNetwork.NetworkInformation.Reachability.Online || QtNetwork.NetworkInformation.reachability === QtNetwork.NetworkInformation.Reachability.Unknown
                        shortcut: StandardKey.Refresh
                        onTriggered: LiveDataManager.checkForUpdates(root.tripGroup.elements);
                    }
                    visible: !root.isEmptyTripGroup && !root.controller.isPastTrip
                }
                FormCard.FormButtonDelegate {
                    text: i18n("Rename…")
                    icon.name: "edit-rename"
                    onClicked: {
                        tripGroupEditor.tripGroupId = root.tripGroupId;
                        tripGroupEditor.tripGroup = root.tripGroup;
                        tripGroupEditor.open();
                    }
                }
                FormCard.FormButtonDelegate {
                    text: i18n("Merge trips…")
                    icon.name: "merge"
                    enabled: root.controller.canMerge
                    visible: !root.isEmptyTripGroup
                    onClicked: {
                        tripGroupMergeDialog.tripGroupId = root.tripGroupId;
                        tripGroupMergeDialog.tripGroup = root.tripGroup;
                        tripGroupMergeDialog.open();
                    }
                }
                FormCard.FormButtonDelegate {
                    text: i18n("Split trip…")
                    icon.name: "split"
                    enabled: root.controller.canSplit
                    visible: !root.isEmptyTripGroup
                    onClicked: applicationWindow().pageStack.push(tripGroupSplitPage, { tripGroup: root.tripGroup });
                }
                FormCard.FormButtonDelegate {
                    text: i18n("Download Maps")
                    icon.name: "download"
                    icon.color: QtNetwork.NetworkInformation.isMetered ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.textColor
                    enabled: QtNetwork.NetworkInformation.reachability === QtNetwork.NetworkInformation.Reachability.Online || QtNetwork.NetworkInformation.reachability === QtNetwork.NetworkInformation.Reachability.Unknown
                    visible: !root.isEmptyTripGroup && !root.controller.isPastTrip
                    onClicked: MapDownloadManager.download(root.tripGroup.elements);
                }
                FormCard.FormButtonDelegate {
                    text: i18n("Export…")
                    icon.name: "document-export-symbolic"
                    enabled: !root.isEmptyTripGroup
                    visible: !root.isEmptyTripGroup
                    onClicked: exportTripGroupDialog.open()
                }
                FormCard.FormButtonDelegate {
                    text: i18n("Delete trip")
                    icon.name: "edit-delete"
                    onClicked: deleteTripGroupWarningDialog.open()
                }
            }
            // spacer for the floating buttons
            Item {
                visible: floatingButtons.visible
                height: root.width < Kirigami.Units.gridUnit * 30 + floatingButtons.width * 2 ? floatingButtons.height : 0
            }
        }
    }

    Components.DoubleFloatingButton {
        id: floatingButtons
        parent: root.overlay
        visible: !root.isEmptyTripGroup
        anchors {
            right: parent.right
            rightMargin: Kirigami.Settings.isMobile ? Kirigami.Units.largeSpacing : Kirigami.Units.largeSpacing + (root.contentItem.QQC2.ScrollBar && root.contentItem.QQC2.ScrollBar.vertical ? root.contentItem.QQC2.ScrollBar.vertical.width : 0)
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.largeSpacing
        }

        leadingAction: Kirigami.Action {
            text: i18nc("@action:button", "Go To Now")
            icon.name: "view-calendar-day"
            onTriggered: listView.positionViewAtIndex(listView.model.todayRow, ListView.Beginning);
            tooltip: text
            enabled: listView.model?.todayRow >= 0 ?? false
        }

        trailingAction: Kirigami.Action{
            text: i18nc("@action:button", "Add trip")
            icon.name: "list-add-symbolic"
            onTriggered: addMenu.popup()
            tooltip: text
        }
    }

    onTripGroupIdChanged: {
        ApplicationController.contextTripGroupId = root.tripGroupId
    }

    // work around initial positioning not working correctly below, as at that point
    // listView.height has bogus values. No idea why, possibly delayed layouting in the ScrollablePage,
    // or a side-effect of the binding loop on delegate heights
    Timer {
        id: positionTimer
        interval: 10
        repeat: false
        onTriggered: listView.positionViewAtIndex(timelineModel.todayRow, ListView.Beginning);
    }

    Component.onCompleted: positionTimer.start()
}
