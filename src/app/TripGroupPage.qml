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
import org.kde.itinerary

Kirigami.ScrollablePage {
    id: root

    required property var tripGroup
    required property string tripGroupId

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

    ListView {
        id: listView
        topMargin: Kirigami.Units.gridUnit
        spacing: Kirigami.Units.gridUnit

        /*model: TripTimelineModel {
            tripGroup: root.tripGroup
            reservationManager: ReservationManager
            transferManager: TransferManager
            tripGroupManager: TripGroupManager
        }*/

        delegate: Models.DelegateChooser {
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
                    onClicked: applicationWindow().pageStack.push(tripGroupSplitPage, { tripGroup: root.tripGroup });
                }
                FormCard.FormButtonDelegate {
                    text: i18n("Export…")
                    icon.name: "export-symbolic"
                    onClicked: exportTripGroupDialog.open()
                }
                FormCard.FormButtonDelegate {
                    text: i18n("Delete trip")
                    icon.name: "edit-delete"
                    onClicked: deleteTripGroupWarningDialog.open()
                }
            }
        }
    }
}
