/*
    SPDX-FileCopyrightText: 2019-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
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
