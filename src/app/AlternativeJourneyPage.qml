/*
    SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kpublictransport
import org.kde.itinerary

JourneyQueryPage {
    id: root

    required property QtObject controller

    title: i18n("Alternative Connections")

    journeyRequest: controller.journeyRequestFull

    function updateRequest() {
        root.journeyRequest = controller.journeyRequest;
        root.journeyRequest.to = controller.journeyDestinations[destinationCombo.currentIndex];

        let allLineModes = true;
        for (const s of [longDistanceModeAction, localTrainModeAction, rapidTransitModeAction, busModeAction, ferryModeAction, aircraftModeAction]) {
            if (!s.checked) {
                allLineModes = false;
            }
        }

        let lineModes = [];
        if (!allLineModes) {
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
                lineModes.push(Line.Air)
        }
        root.journeyRequest.lineModes = lineModes;
    }

    onJourneyChanged: replaceWarningDialog.open()

    Component.onCompleted: {
        destinationCombo.currentIndex = destinationCombo.count - 1
        updateRequest()
    }

    actions: [
        Kirigami.Action {
            id: longDistanceModeAction
            text: i18nc("journey query search constraint, title", "Long distance trains")
            icon.source: LineMode.iconName(Line.LongDistanceTrain)
            checkable: true
            checked: true
            onTriggered: root.updateRequest()
        },
        Kirigami.Action {
            id: localTrainModeAction
            text: i18nc("journey query search constraint, title", "Local trains")
            icon.source: LineMode.iconName(Line.LocalTrain)
            checkable: true
            checked: true
            onTriggered: root.updateRequest()
        },
        Kirigami.Action {
            id: rapidTransitModeAction
            text: i18nc("journey query search constraint, title", "Rapid transit")
            icon.source: LineMode.iconName(Line.Tramway)
            checkable: true
            checked: true
            onTriggered: root.updateRequest()
        },
        Kirigami.Action {
            id: busModeAction
            text: i18nc("journey query search constraint, title", "Bus")
            icon.source: LineMode.iconName(Line.Bus)
            checkable: true
            checked: true
            onTriggered: root.updateRequest()
        },
        Kirigami.Action {
            id: ferryModeAction
            text: i18nc("journey query search constraint, title", "Ferry")
            icon.source: LineMode.iconName(Line.Ferry)
            checkable: true
            checked: true
            onTriggered: root.updateRequest()
        },
        Kirigami.Action {
            id: aircraftModeAction
            text: i18nc("journey query search constraint, title", "Aircraft")
            icon.source: LineMode.iconName(Line.Air)
            checkable: true
            checked: false
            onTriggered: root.updateRequest()
        }
    ]

    data: [
        Kirigami.PromptDialog {
            id: replaceWarningDialog

            title: i18n("Replace Journey")
            subtitle: i18n("Do you really want to replace your existing reservation with the newly selected journey?")
            standardButtons: QQC2.Dialog.No
            customFooterActions: [
                Kirigami.Action {
                    text: i18n("Replace")
                    icon.name: "document-save"
                    onTriggered: {
                        root.controller.applyJourney(root.journey, destinationCombo.currentIndex);
                        replaceWarningDialog.close()
                        applicationWindow().pageStack.pop();
                    }
                }
            ]
        }
    ]

    header: ColumnLayout {
        FormCard.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            FormCard.FormComboBoxDelegate {
                id: destinationCombo
                text: i18n("Destination:")
                model: root.controller.journeyDestinations
                displayText: currentValue.name
                textRole: "name"
                onCurrentIndexChanged: root.updateRequest()
                //BEGIN workaround for Kirigami-Addons < 1.8.2, TODO remove once we depend on that
                comboBoxDelegate: Delegates.RoundedItemDelegate {
                    required property var model
                    required property int index
                    implicitWidth: ListView.view ? ListView.view.width : Kirigami.Units.gridUnit * 16
                    text: model[destinationCombo.textRole]
                    highlighted: destinationCombo.highlightedIndex === index
                }
                dialogDelegate: Delegates.RoundedItemDelegate {
                    required property var model
                    required property int index

                    implicitWidth: ListView.view ? ListView.view.width : Kirigami.Units.gridUnit * 16
                    text: model[destinationCombo.textRole]

                    Layout.topMargin: index == 0 ? Math.round(Kirigami.Units.smallSpacing / 2) : 0

                    onClicked: {
                        destinationCombo.currentIndex = index;
                        destinationCombo.activated(index);
                        destinationCombo.closeDialog();
                    }
                }
                //END
            }
        }
    }
}
