/*
    SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport
import org.kde.itinerary

JourneyQueryPage {
    id: root

    required property QtObject controller

    title: i18n("Alternative Connections")

    journeyRequest: controller.journeyRequestFull

    function updateRequest() {
        root.journeyRequest = fullJourneyAction.checked ? controller.journexRequestFull : controller.journeyRequestOne;

        let allLineModes = true;
        for (const s of [longDistanceModeAction, localTrainModeAction, rapidTransitModeAction, busModeAction, ferryModeAction]) {
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
        }
        root.journeyRequest.lineModes = lineModes;
    }

    onJourneyChanged: replaceWarningDialog.open()

    Component.onCompleted: {
        for (const action of [fullJourneyAction, oneJourneyAction, actionSeparator, longDistanceModeAction, localTrainModeAction, rapidTransitModeAction, busModeAction, ferryModeAction]) {
                actions.push(action);
        }
    }

    data: [
        QQC2.ActionGroup { id: journeyActionGroup },
        Kirigami.Action {
            id: fullJourneyAction
            text: i18nc("to travel destination", "To %1", root.controller.journeyRequestFull.to.name)
            checkable: true
            checked: root.controller.journeyRequestFull.to.name == root.journeyRequest.to.name
            icon.name: "go-next-symbolic"
            visible: root.controller.journeyRequestFull.to.name != root.controller.journeyRequestOne.to.name
            QQC2.ActionGroup.group: journeyActionGroup
            onTriggered: root.updateRequest()
        },
        Kirigami.Action {
            id: oneJourneyAction
            text: i18nc("to travel destination", "To %1", root.controller.journeyRequestOne.to.name)
            checkable: true
            checked: root.controller.journeyRequestOne.to.name == root.journeyRequest.to.name
            icon.name: "go-next-symbolic"
            visible: root.controller.journeyRequestFull.to.name != root.controller.journeyRequestOne.to.name
            QQC2.ActionGroup.group: journeyActionGroup
            onTriggered: root.updateRequest()
        },

        Kirigami.Action {
            id: actionSeparator
            separator: true
        },

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
                        root.controller.applyJourney(root.journey, root.journeyRequest.to.name == root.controller.journeyRequestFull.to.name);
                        replaceWarningDialog.close()
                        applicationWindow().pageStack.pop();
                    }
                }
            ]
        }
    ]
}
