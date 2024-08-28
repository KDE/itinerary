// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtCore as QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import Qt.labs.qmlmodels as Models
import org.kde.i18n.localeData
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components as Components
import org.kde.kpublictransport as KPublicTransport
import org.kde.itinerary

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
    }

    title: tripGroup.name

    Kirigami.Theme.colorSet: Kirigami.Theme.Window

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
        const place = TripGroupModel.locationAtTime(dt);
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
            icon.name: KPublicTransport.LineMode.iconName(KPublicTransport.Line.Air)
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
            icon.name: KPublicTransport.LineMode.iconName(KPublicTransport.Line.Ferry)
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

    SheetDrawer {
        id: addMenu
        headerItem: Kirigami.Heading {
            leftPadding: Kirigami.Units.smallSpacing
            text: i18nc("@title:group", "Add to Trip")
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

    Kirigami.MenuDialog {
        id: exportTripGroupDialog
        title: i18n("Export")
        property list<QQC2.Action> _actions: [
            Kirigami.Action {
                text: i18n("As Itinerary file…")
                icon.name: "export-symbolic"
                onTriggered: {
                    tripGroupFileExportDialog.currentFile = root.tripGroup.slugName + ".itinerary"
                    tripGroupFileExportDialog.open();
                }
            },
            Kirigami.Action {
                text: i18n("As GPX file…")
                icon.name: "map-globe"
                onTriggered: {
                    tripGroupGpxExportDialog.currentFile = root.tripGroup.slugName  + ".gpx"
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
                onTriggered: ApplicationController.exportTripToKDEConnect(root.tripGroupId, model.deviceId)
            }
            onObjectAdded: (object) => {
                exportTripGroupDialog._actions.push(object);
            }
        }
        onVisibleChanged: {
            if (exportTripGroupDialog.visible)
                deviceModel.refresh();
        }
    }
    FileDialog {
        id: tripGroupFileExportDialog
        fileMode: FileDialog.SaveFile
        title: i18n("Export Trip")
        nameFilters: [i18n("Itinerary file (*.itinerary)")]
        onAccepted: {
            ApplicationController.exportTripToFile(root.tripGroupId, tripGroupFileExportDialog.selectedFile);
            Settings.writeFileDialogFolder("tripGroupExport", tripGroupFileExportDialog.selectedFile)
        }
        onVisibleChanged: {
            if (tripGroupFileExportDialog.visible) {
                tripGroupFileExportDialog.currentFolder = Settings.readFileDialogFolder("tripGroupExport", QtCore.StandardPaths.writableLocation(QtCore.StandardPaths.DocumentsLocation));
            }
        }
    }
    FileDialog {
        id: tripGroupGpxExportDialog
        fileMode: FileDialog.SaveFile
        title: i18n("Export Trip")
        currentFolder: QtCore.StandardPaths.writableLocation(QtCore.StandardPaths.DocumentsLocation)
        nameFilters: [i18n("GPX Files (*.gpx)")]
        onAccepted: {
            ApplicationController.exportTripToGpx(root.tripGroupId, tripGroupGpxExportDialog.selectedFile);
            Settings.writeFileDialogFolder("tripGroupGpxExport", tripGroupGpxExportDialog.selectedFile)
        }
        onVisibleChanged: {
            if (tripGroupGpxExportDialog.visible) {
                tripGroupGpxExportDialog.currentFolder = Settings.readFileDialogFolder("tripGroupGpxExport", QtCore.StandardPaths.writableLocation(QtCore.StandardPaths.DocumentsLocation));
            }
        }
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
        spacing: Kirigami.Units.gridUnit

        header: ColumnLayout {
            width: listView.width

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

                FormCard.FormTextDelegate {
                    visible: root.controller.weatherForecast.valid
                    icon.name:  root.controller.weatherForecast.symbolIconName
                    text: i18nc("temperature range", "%1 / %2",  Localizer.formatTemperature(root.controller.weatherForecast.minimumTemperature),
                                                                 Localizer.formatTemperature(root.controller.weatherForecast.maximumTemperature))
                }

                Repeater {
                    model: root.controller.locationInformation

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

                FormCard.FormTextDelegate {
                    icon.name: "view-currency-list"
                    text: root.controller.currencies.length > 0 ? i18np("Currency: %2", "Currencies: %2", root.controller.currencies.length, root.controller.currencies.join(", ")) : ""
                    visible: root.controller.currencies.length > 0
                }
            }

            FormCard.FormHeader {
                title: i18n("Import")
                visible: root.isEmptyTripGroup
            }
            FormCard.FormCard {
                visible: root.isEmptyTripGroup
                Repeater {
                    model: importActions
                    FormCard.FormButtonDelegate {
                        action: modelData
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
                    model: root.addActions
                    FormCard.FormButtonDelegate {
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
            labelPositioning: ViewSection.CurrentLabelAtStart | ViewSection.InlineLabels
        }

        delegate: Models.DelegateChooser {
            role: "type"
            Models.DelegateChoice {
                roleValue: TimelineElement.Flight
                FlightDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: flightDetailsPage
                }
            }
            Models.DelegateChoice {
                roleValue: TimelineElement.Hotel
                HotelDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: hotelDetailsPage
                }
            }
            Models.DelegateChoice {
                roleValue: TimelineElement.TrainTrip
                TrainDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: trainDetailsPage
                }
            }
            Models.DelegateChoice {
                roleValue: TimelineElement.BusTrip
                BusDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: busDetailsPage
                }
            }
            Models.DelegateChoice {
                roleValue: TimelineElement.Restaurant
                RestaurantDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: restaurantDetailsPage
                }
            }
            Models.DelegateChoice {
                roleValue: TimelineElement.TouristAttraction
                TouristAttractionDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: touristAttractionDetailsPage
                }
            }
            Models.DelegateChoice {
                roleValue: TimelineElement.Event
                EventDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: eventDetailsPage
                }
            }
            Models.DelegateChoice {
                roleValue: TimelineElement.CarRental
                CarRentalDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: carRentalDetailsPage
                }
            }
            Models.DelegateChoice {
                roleValue: TimelineElement.BoatTrip
                BoatDelegate {
                    batchId: model.batchId
                    rangeType: model.rangeType
                    detailsPage: boatDetailsPage
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
                roleValue: TimelineElement.Transfer
                TransferDelegate {}
            }
        }

        footer: ColumnLayout {
            width: listView.width

            FormCard.FormHeader {
                title: i18n("Actions")
            }

            FormCard.FormCard {
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
                    text: i18n("Export…")
                    icon.name: "export-symbolic"
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
            enabled: listView.model.todayRow >= 0
        }

        trailingAction: Kirigami.Action{
            text: i18nc("@action:button", "Add trip")
            icon.name: "list-add-symbolic"
            onTriggered: addMenu.open()
            tooltip: text
        }
    }

}
