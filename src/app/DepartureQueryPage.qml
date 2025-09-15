/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kpublictransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.itinerary

Kirigami.ScrollablePage {
    id: root

    property var stop
    property var dateTime

    title: i18n("Departures")
    Kirigami.Theme.colorSet: Kirigami.Theme.Window

    actions: [
        Kirigami.Action {
            id: longDistanceModeAction
            text: i18nc("journey query search constraint, title", "Long distance trains")
            icon.source: LineMode.iconName(Line.LongDistanceTrain)
            displayHint: Kirigami.DisplayHint.AlwaysHide
            checkable: true
            checked: true
        },
        Kirigami.Action {
            id: localTrainModeAction
            text: i18nc("journey query search constraint, title", "Local trains")
            icon.source: LineMode.iconName(Line.LocalTrain)
            displayHint: Kirigami.DisplayHint.AlwaysHide
            checkable: true
            checked: true
        },
        Kirigami.Action {
            id: rapidTransitModeAction
            text: i18nc("journey query search constraint, title", "Rapid transit")
            icon.source: LineMode.iconName(Line.Tramway)
            displayHint: Kirigami.DisplayHint.AlwaysHide
            checkable: true
            checked: true
        },
        Kirigami.Action {
            id: busModeAction
            text: i18nc("journey query search constraint, title", "Bus")
            icon.source: LineMode.iconName(Line.Bus)
            displayHint: Kirigami.DisplayHint.AlwaysHide
            checkable: true
            checked: true
        },
        Kirigami.Action {
            id: ferryModeAction
            text: i18nc("journey query search constraint, title", "Ferry")
            icon.source: LineMode.iconName(Line.Ferry)
            displayHint: Kirigami.DisplayHint.AlwaysHide
            checkable: true
            checked: true
        },
        Kirigami.Action {
            id: aircraftModeAction
            text: i18nc("journey query search constraint, title", "Aircraft")
            icon.source: LineMode.iconName(Line.Air)
            displayHint: Kirigami.DisplayHint.AlwaysHide
            checkable: true
            checked: false
        }
    ]

    function allLineModes()
    {
        for (const s of [longDistanceModeAction, localTrainModeAction, rapidTransitModeAction, busModeAction, ferryModeAction, aircraftModeAction]) {
            if (!s.checked) {
                return false;
            }
        }
        return true;
    }

    function stopoverRequest()
    {
        let req = PublicTransport.stopoverRequestForPlace(stop, dateTime);
        req.downloadAssets = Settings.wikimediaOnlineContentEnabled;
        let lineModes = [];
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
            if (aircraftModeAction.checked)
                lineModes.push(Line.Air);
        }
        req.lineModes = lineModes;
        return req;
    }

    StopoverQueryModel {
        id: departureModel
        manager: LiveDataManager.publicTransportManager
        request: stopoverRequest();
        autoUpdate: true
    }

    ListView {
        id: journeyView
        anchors.fill: parent
        clip: true
        model: departureModel
        spacing: Kirigami.Units.largeSpacing

        header: VerticalNavigationButton {
            visible: departureModel.canQueryPrevious
            width: journeyView.width
            text: i18nc("@action:button", "Load earlier departures")
            iconName: "go-up-symbolic"
            onClicked: departureModel.queryPrevious()
        }

        delegate: FormCard.FormCard {
            id: delegateRoot
            required property stopover stopover
            width: ListView.view.width
            StopoverFormDelegate {
                stopover: delegateRoot.stopover
                onClicked: {
                    if (delegateRoot.stopover.stopPoint.hasCoordinate || delegateRoot.stopover.hasTripIdentifiers) {
                        applicationWindow().pageStack.push(Qt.createComponent('org.kde.itinerary', 'StopoverDetailsPage'), {
                            stopover: delegateRoot.stopover,
                            ptMgr: LiveDataManager.publicTransportManager
                        });
                    }
                }
            }
        }

        footer: VerticalNavigationButton {
            visible: departureModel.canQueryNext
            width: journeyView.width
            iconName: "go-down-symbolic"
            text: i18nc("@action:button", "Load later connections")
            onClicked: departureModel.queryNext()

            FormCard.FormCard {
                visible: departureModel.attributions.length > 0

                Layout.fillWidth: true

                FormCard.FormTextDelegate {
                    text: i18n("Data providers:")
                    description: PublicTransport.attributionSummary(departureModel.attributions)
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }
        }

        Kirigami.LoadingPlaceholder {
            text: departureModel.loading ? i18nc("@info:placeholder", "Loading") : i18nc("@info:status", "Error")
            visible: departureModel.loading || explanation.length > 0

            icon.name: !departureModel.loading ? "network-disconnect-symbolic" : ""
            explanation: departureModel.errorMessage
            progressBar.visible: departureModel.loading
            anchors.centerIn: parent
            width: parent.width - Kirigami.Units.gridUnit * 4
        }
    }
}
