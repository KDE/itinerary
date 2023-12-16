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
import "." as App

Kirigami.OverlaySheet {
    id: boardSheet
    property alias title: headerLabel.text
    property alias action: footerBottom.action
    property alias model: stopSelector.model
    property alias currentIndex: stopSelector.currentIndex

    header: Kirigami.Heading {
        id: headerLabel
    }

    ListView {
        id: stopSelector
        currentIndex: -1
        delegate: Kirigami.BasicListItem {
            highlighted: ListView.isCurrentItem
            text: {
                if (modelData.scheduledDepartureTime.getTime()) {
                    return Localizer.formatTime(modelData, "scheduledDepartureTime") + " " + modelData.stopPoint.name
                }
                return Localizer.formatTime(modelData, "scheduledArrivalTime") + " " + modelData.stopPoint.name
            }
            enabled: modelData.disruptionEffect != Disruption.NoService
        }
    }

    footer: QQC2.Button {
        id: footerBottom
        enabled: stopSelector.currentIndex >= 0
    }
}
