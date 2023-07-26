/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kitemmodels 1.0
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import "." as App

Kirigami.ScrollablePage {
    id: root
    property var transfer
    readonly property var reservation: ReservationManager.reservation(transfer.reservationId)
    title: i18n("Select Transfer")

    JourneyQueryModel {
        id: journeyModel
        manager: LiveDataManager.publicTransportManager
    }

    KSortFilterProxyModel {
        id: sortedJourneyModel
        sourceModel: journeyModel
        sortRole: JourneyQueryModel.ScheduledDepartureTime
        dynamicSortFilter: true
        Component.onCompleted: Util.sortModel(sortedJourneyModel, 0, Qt.Ascending)
    }

    background: Rectangle {
        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
    }

    actions.contextualActions: [
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
            icon.source: "qrc:///images/bike.svg"
            checkable: true
            onCheckedChanged: queryJourney()
        },
        Kirigami.Action {
            id: bikeRideAction
            text: i18n("Bike & Ride")
            icon.source: "qrc:///images/bike.svg"
            checkable: true
            visible: root.transfer.alignment == Transfer.Before && root.transfer.floatingLocationType == Transfer.FavoriteLocation
            onCheckedChanged: queryJourney()
        },
        Kirigami.Action {
            id: bikeSharingAction
            text: i18n("Shared Bikes")
            icon.source: "qrc:///images/bike.svg"
            checkable: true
            onCheckedChanged: queryJourney()
        },
        Kirigami.Action {
            id: parkRideAction
            text: i18n("Park & Ride")
            icon.source: "qrc:///images/car.svg"
            checkable: true
            visible: root.transfer.alignment == Transfer.Before && root.transfer.floatingLocationType == Transfer.FavoriteLocation
            onCheckedChanged: queryJourney()
        },

        Kirigami.Action { separator: true },

        Kirigami.Action {
            id: longDistanceModeAction
            text: i18nc("journey query search constraint, title", "Long distance trains")
            icon.source: PublicTransport.lineModeIcon(Line.LongDistanceTrain)
            checkable: true
            checked: true
            onCheckedChanged: queryJourney()
        },
        Kirigami.Action {
            id: localTrainModeAction
            text: i18nc("journey query search constraint, title", "Local trains")
            icon.source: PublicTransport.lineModeIcon(Line.LocalTrain)
            checkable: true
            checked: true
            onCheckedChanged: queryJourney()
        },
        Kirigami.Action {
            id: rapidTransitModeAction
            text: i18nc("journey query search constraint, title", "Rapid transit")
            icon.source: PublicTransport.lineModeIcon(Line.Tramway)
            checkable: true
            checked: true
            onCheckedChanged: queryJourney()
        },
        Kirigami.Action {
            id: busModeAction
            text: i18nc("journey query search constraint, title", "Bus")
            icon.source: PublicTransport.lineModeIcon(Line.Bus)
            checkable: true
            checked: true
            onCheckedChanged: queryJourney()
        },
        Kirigami.Action {
            id: ferryModeAction
            text: i18nc("journey query search constraint, title", "Ferry")
            icon.source: PublicTransport.lineModeIcon(Line.Ferry)
            checkable: true
            checked: true
            onCheckedChanged: queryJourney()
        }
    ]

    function allLineModes()
    {
        for (const s of [longDistanceModeAction, localTrainModeAction, rapidTransitModeAction, busModeAction, ferryModeAction]) {
            if (!s.checked) {
                return false;
            }
        }
        return true;
    }

    function queryJourney() {
        journeyModel.request = TransferManager.journeyRequestForTransfer(transfer);
        let lineModes = [];
        let accessMode = [];
        let egressMode = [];

        if (!allLineModes()) {
            if (longDistanceModeAction.checked)
                lineModes.push(Line.LongDistanceTrain, Line.Train);
            if (localTrainModeAction.checked)
                lineModes.push(Line.LocalTrain);
            if (rapidTransitModeAction.checked)
                lineModes.push(Line.RapidTransit, Line.Metro, Line.Tramway, Line.RailShuttle);
            if (busModeAction.checked)
                lineModes.push(Line.Bus, Line.Coach);
            if (ferryModeAction.checked)
                lineModes.push(Line.Ferry, Line.Boat);
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

    Component {
        id: journeyDelegate

        MobileForm.FormCard {
            id: top

            required property int index
            required property var journey

            width: ListView.view.width

            contentItem: ColumnLayout {
                id: contentLayout

                spacing: 0

                JourneyDelegateHeader {
                    journey: top.journey
                }

                Repeater {
                    delegate: App.JourneySectionDelegate{
                        Layout.fillWidth: true
                    }
                    model: journeyView.currentIndex === top.index ? top.journey.sections : 0
                }

                App.JourneySummaryDelegate {
                    id: summaryButton

                    journey: top.journey
                    visible: journeyView.currentIndex !== top.index
                    onClicked: journeyView.currentIndex = top.index

                    Layout.fillWidth: true
                }

                MobileForm.FormDelegateSeparator {
                    above: selectButton
                    visible: journeyView.currentIndex === top.index
                }

                MobileForm.FormButtonDelegate {
                    id: selectButton

                    text: i18n("Select")
                    icon.name: "checkmark"
                    visible: journeyView.currentIndex === top.index
                    enabled: top.journey.disruptionEffect !== Disruption.NoService
                    onClicked: {
                        TransferManager.setJourneyForTransfer(root.transfer, top.journey);
                        applicationWindow().pageStack.pop();
                    }
                }
            }
        }
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

    ListView {
        id: journeyView

        clip: true
        delegate: journeyDelegate
        model: sortedJourneyModel
        spacing: Kirigami.Units.largeSpacing

        header: VerticalNavigationButton {
            visible: journeyModel.canQueryPrevious
            width: journeyView.width
            text: i18nc("@action:button", "Load earlier connections")
            iconName: "go-up-symbolic"
            onClicked: journeyModel.queryPrevious()
        }

        footer: VerticalNavigationButton {
            visible: journeyModel.canQueryNext
            width: journeyView.width
            iconName: "go-down-symbolic"
            text: i18nc("@action:button", "Load later connections")
            onClicked: journeyModel.queryNext()

            MobileForm.FormCard {
                visible: journeyModel.attributions.length > 0

                Layout.fillWidth: true

                contentItem: ColumnLayout {
                    spacing: 0

                    MobileForm.FormTextDelegate {
                        text: i18n("Data providers:")
                        description: PublicTransport.attributionSummary(journeyModel.attributions)
                        onLinkActivated: Qt.openUrlExternally(link)
                    }
                }
            }
        }

        QQC2.BusyIndicator {
            anchors.centerIn: parent
            running: journeyModel.loading
        }

        QQC2.Label {
            anchors.centerIn: parent
            width: parent.width - Kirigami.Units.gridUnit * 4
            text: journeyModel.errorMessage
            color: Kirigami.Theme.negativeTextColor
            wrapMode: Text.Wrap
        }
    }
}
