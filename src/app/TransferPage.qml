/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kitemmodels
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary
import org.kde.kirigamiaddons.formcard as FormCard

PublicTransport.AbstractJourneySelectionPage {
    id: root

    property var transfer
    readonly property var reservation: ReservationManager.reservation(transfer.reservationId)

    title: i18n("Select Transfer")

    journeyModel: KPublicTransport.JourneyQueryModel {
        id: journeyModel
        manager: LiveDataManager.publicTransportManager
    }

    actions: [
        Kirigami.Action {
            text: i18n("Depart Now")
            icon.name: "clock"
            onTriggered: {
                var req = journeyModel.request;
                req.dateTime = new Date();
                req.dateTimeMode = JourneyRequest.Departure;
                journeyModel.request = req;
            }
        },
        Kirigami.Action {
            text: i18n("Discard")
            icon.name: "edit-delete"
            onTriggered: {
                TransferManager.discardTransfer(root.transfer);
                applicationWindow().pageStack.pop();
            }
        },
        Kirigami.Action {
            text: i18n("Earlier")
            icon.name: "go-up-symbolic"
            onTriggered: journeyModel.queryPrevious()
            enabled: journeyModel.canQueryPrevious
        },
        Kirigami.Action {
            text: i18n("Later")
            icon.name: "go-down-symbolic"
            onTriggered: journeyModel.queryNext()
            enabled: journeyModel.canQueryNext
        },
        Kirigami.Action { separator: true },
        Kirigami.Action {
            id: bikeAction
            text: i18n("Bike")
            icon.source: KPublicTransport.IndividualTransportMode.modeIconName(IndividualTransport.Bike)
            checkable: true
            onCheckedChanged: queryJourney()
        },
        Kirigami.Action {
            id: bikeRideAction
            text: i18n("Bike & Ride")
            icon.source: KPublicTransport.IndividualTransportMode.modeIconName(IndividualTransport.Bike)
            checkable: true
            visible: root.transfer.alignment == Transfer.Before && root.transfer.floatingLocationType == Transfer.FavoriteLocation
            onCheckedChanged: queryJourney()
        },
        Kirigami.Action {
            id: bikeSharingAction
            text: i18n("Shared Bikes")
            icon.source: KPublicTransport.RentalVehicleType.vehicleTypeIconName(RentalVehicle.Bicycle)
            checkable: true
            onCheckedChanged: queryJourney()
        },
        Kirigami.Action {
            id: parkRideAction
            text: i18n("Park & Ride")
            icon.source: KPublicTransport.RentalVehicleType.vehicleTypeIconName(RentalVehicle.Car)
            checkable: true
            visible: root.transfer.alignment == Transfer.Before && root.transfer.floatingLocationType == Transfer.FavoriteLocation
            onCheckedChanged: queryJourney()
        },

        Kirigami.Action { separator: true },

        Kirigami.Action {
            id: longDistanceModeAction
            text: i18nc("journey query search constraint, title", "Long distance trains")
            icon.source: KPublicTransport.LineMode.iconName(KPublicTransport.Line.LongDistanceTrain)
            checkable: true
            checked: true
            onCheckedChanged: queryJourney()
        },
        Kirigami.Action {
            id: localTrainModeAction
            text: i18nc("journey query search constraint, title", "Local trains")
            icon.source: KPublicTransport.LineMode.iconName(KPublicTransport.Line.LocalTrain)
            checkable: true
            checked: true
            onCheckedChanged: queryJourney()
        },
        Kirigami.Action {
            id: rapidTransitModeAction
            text: i18nc("journey query search constraint, title", "Rapid transit")
            icon.source: KPublicTransport.LineMode.iconName(KPublicTransport.Line.Tramway)
            checkable: true
            checked: true
            onCheckedChanged: queryJourney()
        },
        Kirigami.Action {
            id: busModeAction
            text: i18nc("journey query search constraint, title", "Bus")
            icon.source: KPublicTransport.LineMode.iconName(KPublicTransport.Line.Bus)
            checkable: true
            checked: true
            onCheckedChanged: queryJourney()
        },
        Kirigami.Action {
            id: ferryModeAction
            text: i18nc("journey query search constraint, title", "Ferry")
            icon.source: KPublicTransport.LineMode.iconName(KPublicTransport.Line.Ferry)
            checkable: true
            checked: true
            onCheckedChanged: queryJourney()
        }
    ]

    function allLineModes(): bool {
        for (const s of [longDistanceModeAction, localTrainModeAction, rapidTransitModeAction, busModeAction, ferryModeAction]) {
            if (!s.checked) {
                return false;
            }
        }
        return true;
    }

    function queryJourney(): void {
        journeyModel.request = TransferManager.journeyRequestForTransfer(transfer);
        let lineModes = [];
        let accessMode = [];
        let egressMode = [];

        if (!allLineModes()) {
            if (longDistanceModeAction.checked)
                lineModes.push(PublicTransport.Line.LongDistanceTrain, PublicTransport.Line.Train);
            if (localTrainModeAction.checked)
                lineModes.push(PublicTransport.Line.LocalTrain);
            if (rapidTransitModeAction.checked)
                lineModes.push(PublicTransport.Line.RapidTransit, PublicTransport.Line.Metro, PublicTransport.Line.Tramway, PublicTransport.Line.RailShuttle);
            if (busModeAction.checked)
                lineModes.push(PublicTransport.Line.Bus, PublicTransport.Line.Coach);
            if (ferryModeAction.checked)
                lineModes.push(PublicTransport.Line.Ferry, PublicTransport.Line.Boat);
        }

        if (bikeAction.checked) {
            accessMode.push({ mode: IndividualTransport.Bike });
            egressMode.push({ mode: IndividualTransport.Bike });
        }
        if (bikeRideAction.checked) {
            accessMode.push({ mode: IndividualTransport.Bike, qualifier: IndividualTransport.Park });
        }
        if (bikeSharingAction.checked) {
            accessMode.push({ mode: IndividualTransport.Bike, qualifier: IndividualTransport.Rent });
            egressMode.push({ mode: IndividualTransport.Bike, qualifier: IndividualTransport.Rent });
        }

        if (parkRideAction.checked) {
            accessMode.push({ mode: IndividualTransport.Car, qualifier: IndividualTransport.Park });
        }

        journeyModel.request.lineModes = lineModes;
        journeyModel.request.accessModes = accessMode;
        journeyModel.request.egressModes = egressMode;
    }

    Component.onCompleted: {
        if (transfer.floatingLocationType == Transfer.FavoriteLocation) {
            favLocCombo.currentIndex = favLocCombo.find(transfer.alignment == Transfer.Before ? transfer.fromName : transfer.toName);
        }
        queryJourney();
    }

    header: QQC2.Pane {
        Kirigami.Theme.colorSet: Kirigami.Theme.Header
        Kirigami.Theme.inherit: false

        contentItem: ColumnLayout {
            id: topLayout

            QQC2.Label {
                text: {
                    if (!Util.isLocationChange(root.reservation)) {
                        return i18n("%1 ends at %2", root.reservation.reservationFor.name, Localizer.formatTime(transfer, "anchorTime"));
                    }
                    return i18n("Preceding arrival %1 at %2", Localizer.formatTime(transfer, "anchorTime"), transfer.fromName);
                }
                visible: transfer.alignment === Transfer.After
                Layout.fillWidth: true
            }

            QQC2.Label {
                Layout.fillWidth: true
                text: {
                    if (!Util.isLocationChange(root.reservation)) {
                        return i18n("%1 starts at %2", root.reservation.reservationFor.name, Localizer.formatTime(transfer, "anchorTime"));
                    }
                    return i18n("Following departure %1 from %2", Localizer.formatTime(transfer, "anchorTime"), transfer.toName);
                }
                visible: transfer.alignment === Transfer.Before
            }

            QQC2.ComboBox {
                id: favLocCombo
                model: FavoriteLocationModel
                visible: transfer.floatingLocationType === Transfer.FavoriteLocation
                textRole: "display"
                Layout.fillWidth: true
                onActivated: {
                    var favLoc = delegateModel.items.get(currentIndex)
                    console.log(favLoc.model.favoriteLocation);
                    root.transfer = TransferManager.setFavoriteLocationForTransfer(root.transfer, favLoc.model.favoriteLocation);
                    queryJourney();
                }
            }
        }
    }
}
