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
        return listView.model.index(row, 0);
    }

    /** Date/time the given model index refers to, depending on the type of the element this refers to. */
    function dateTimeAtIndex(idx) {
        if (listView.model.data(idx, TimelineModel.IsTimeboxedRole) && !listView.model.data(idx, TimelineModel.IsCanceledRole)) {
            return listView.model.data(idx, TimelineModel.EndDateTimeRole);
        }
        return listView.model.data(idx, TimelineModel.StartDateTimeRole);
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
            onObjectAdded: (index, object) => {
                console.log(object)
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
        id: tripGroupMapPage
        TripGroupMapPage { tripGroupId: root.tripGroupId }
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

                FormCard.FormButtonDelegate {
                    icon.name: "map-globe"
                    text: i18n("Show map…")
                    visible: !root.isEmptyTripGroup
                    onClicked: applicationWindow().pageStack.push(tripGroupMapPage);
                }
            }

            FormCard.FormCard {
                Layout.topMargin: Kirigami.Units.gridUnit

                visible: root.isEmptyTripGroup

                FormCard.AbstractFormDelegate {
                    background: null
                    contentItem: Kirigami.PlaceholderMessage {
                        text: i18nc("@info", "Empty Trip")
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
                title: i18n("Statistics")
                visible: distanceStats.visible || co2Stats.visible || costStats.visible
            }
            FormCard.FormCard {
                visible: distanceStats.visible || co2Stats.visible || costStats.visible
                FormCard.FormTextDelegate {
                    id: distanceStats
                    text: i18n("Distance")
                    description: Localizer.formatDistance(root.controller.totalDistance)
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
                    action: addTripAction
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
            enabled: listView.model?.todayRow >= 0 ?? false
        }

        trailingAction: Kirigami.Action{
            id: addTripAction
            text: i18nc("@action:button", "Add to Trip")
            icon.name: "list-add-symbolic"
            onTriggered: {
                const component = Qt.createComponent("org.kde.itinerary", "TripGroupImportPage");
                root.Window.window.pageStack.push(component, {
                    listView: listView,
                    isEmptyTripGroup: root.isEmptyTripGroup,
                });
            }
            tooltip: text
        }
    }

    Components.FloatingButton {
        parent: root.overlay
        visible: root.isEmptyTripGroup
        anchors {
            right: parent.right
            rightMargin: Kirigami.Settings.isMobile ? Kirigami.Units.largeSpacing : Kirigami.Units.largeSpacing + (root.contentItem.QQC2.ScrollBar && root.contentItem.QQC2.ScrollBar.vertical ? root.contentItem.QQC2.ScrollBar.vertical.width : 0)
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.largeSpacing
        }
        action: addTripAction
    }

    onTripGroupIdChanged: {
        ApplicationController.contextTripGroupId = root.tripGroupId
    }

    // work around initial positioning not working correctly below, as at that point
    // listView.height has bogus values. No idea why, possibly delayed layouting in the ScrollablePage,
    // or a side-effect of the binding loop on delegate heights
    Timer {
        id: positionTimer
        interval: 0
        repeat: false
        onTriggered: listView.positionViewAtIndex(timelineModel.todayRow, ListView.Beginning);
    }

    Component.onCompleted: positionTimer.start()
}
