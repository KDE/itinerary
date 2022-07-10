/*
    SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.19 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

App.JourneyQueryPage {
    id: root

    property QtObject controller;

    title: i18n("Alternative Connections")
    journeyRequest: controller.journeyRequestFull

    onJourneyChanged: replaceWarningDialog.open()

    QQC2.ActionGroup { id: journeyActionGroup }
    Component {
        id: fullJourneyAction
        Kirigami.Action {
            text: i18nc("to travel destination", "To %1", controller.journeyRequestFull.to.name)
            checkable: true
            checked: controller.journeyRequestFull.to.name == root.journeyRequest.to.name
            iconName: "go-next-symbolic"
            visible: controller.journeyRequestFull.to.name != controller.journeyRequestOne.to.name
            QQC2.ActionGroup.group: journeyActionGroup
            onTriggered: root.journeyRequest = controller.journeyRequestFull
        }
    }
    Component {
        id: oneJourneyAction
        Kirigami.Action {
            text: i18nc("to travel destination", "To %1", controller.journeyRequestOne.to.name)
            checkable: true
            checked: controller.journeyRequestOne.to.name == root.journeyRequest.to.name
            iconName: "go-next-symbolic"
            visible: controller.journeyRequestFull.to.name != controller.journeyRequestOne.to.name
            QQC2.ActionGroup.group: journeyActionGroup
            onTriggered: root.journeyRequest = controller.journeyRequestOne
        }
    }

    Component.onCompleted: {
        actions.contextualActions.push(fullJourneyAction.createObject(root));
        actions.contextualActions.push(oneJourneyAction.createObject(root));
    }

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
                    controller.applyJourney(root.journey, root.journeyRequest.to.name == controller.journeyRequestFull.to.name);
                    applicationWindow().pageStack.pop();
                }
            }
        ]
    }
}
