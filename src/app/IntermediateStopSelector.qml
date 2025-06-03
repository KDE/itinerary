/*
    SPDX-FileCopyrightText: 2019-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport
import org.kde.itinerary

Kirigami.Dialog {
    id: boardSheet

    property Kirigami.Action action
    property alias model: stopSelector.model
    property alias currentIndex: stopSelector.currentIndex
    property int disableBeforeIndex: -1
    property int disableAfterIndex: stopSelector.count
    property bool forBoarding: true
    readonly property bool forAlighting: !forBoarding

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
    height: Math.min(applicationWindow().height, Kirigami.Units.gridUnit * 32)

    contentItem: ListView {
        id: stopSelector
        currentIndex: -1
        clip: true
        delegate: QQC2.ItemDelegate {
            id: delegateRoot
            required property KPublicTransport.stopover modelData
            required property int index
            highlighted: ListView.isCurrentItem
            width: ListView.view.width
            contentItem: Kirigami.TitleSubtitle {
                font.strikeout: delegateRoot.modelData.disruptionEffect === KPublicTransport.Disruption.NoService
                title: {
                    if (delegateRoot.modelData.scheduledDepartureTime.getTime()) {
                        return Localizer.formatTime(delegateRoot.modelData, "scheduledDepartureTime") + " " + delegateRoot.modelData.stopPoint.name
                    }
                    return Localizer.formatTime(delegateRoot.modelData, "scheduledArrivalTime") + " " + delegateRoot.modelData.stopPoint.name
                }
            }
            onClicked: ListView.view.currentIndex = index
            enabled: modelData.disruptionEffect != KPublicTransport.Disruption.NoService
                && index > boardSheet.disableBeforeIndex
                && index < boardSheet.disableAfterIndex
                && (!boardSheet.forBoarding || modelData.pickupType !== KPublicTransport.PickupDropoff.NotAllowed)
                && (!boardSheet.forAlighting || modelData.dropoffType !== KPublicTransport.PickupDropoff.NotAllowed)
        }
    }

    customFooterActions: [boardSheet.action]
}
