/*
    SPDX-FileCopyrightText: 2019-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kitinerary
import org.kde.kpublictransport
import org.kde.itinerary

Kirigami.Dialog {
    id: boardSheet

    property Kirigami.Action action
    property alias model: stopSelector.model
    property alias currentIndex: stopSelector.currentIndex

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
    height: Math.min(applicationWindow().height, Kirigami.Units.gridUnit * 32)

    contentItem: ListView {
        id: stopSelector
        currentIndex: -1
        delegate: QQC2.ItemDelegate {
            highlighted: ListView.isCurrentItem
            width: ListView.view.width
            contentItem: Kirigami.TitleSubtitle {
                title: {
                    if (modelData.scheduledDepartureTime.getTime()) {
                        return Localizer.formatTime(modelData, "scheduledDepartureTime") + " " + modelData.stopPoint.name
                    }
                    return Localizer.formatTime(modelData, "scheduledArrivalTime") + " " + modelData.stopPoint.name
                }
            }
            onClicked: ListView.view.currentIndex = index
            enabled: modelData.disruptionEffect != Disruption.NoService
        }
    }

    customFooterActions: [boardSheet.action]
}
