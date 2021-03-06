/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0

Kirigami.ScrollablePage {
    title: i18n("Journey Details")
    property var journeySection

    header: Item {
        height: childrenRect.height + 2 * Kirigami.Units.gridUnit
        GridLayout {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Kirigami.Units.gridUnit

            columns: 2
            columnSpacing: Kirigami.Units.largeSpacing
            rows: 4
            rowSpacing: 0

            Kirigami.Icon {
                Layout.rowSpan: 4
                id: icon
                source: PublicTransport.lineIcon(journeySection.route.line)
                width: height
                height: Kirigami.Units.iconSizes.large
                isMask: !journeySection.route.line.hasLogo && !journeySection.route.line.hasModeLogo
            }

            QQC2.Label {
                Layout.row: 0
                Layout.column: 1
                Layout.fillWidth: true
                text: "<b>" + journeySection.route.line.modeString + " " + journeySection.route.line.name + "</b>"
            }

            QQC2.Label {
                Layout.row: 1
                Layout.column: 1
                Layout.columnSpan: 2
                text: i18n("Direction: %1", journeySection.route.direction)
            }

            QQC2.Label {
                Layout.row: 2
                Layout.column: 1
                Layout.columnSpan: 2
                text: i18n("Distance: %1", Localizer.formatDistance(journeySection.distance))
            }
            QQC2.Label {
                Layout.row: 3
                Layout.column: 1
                Layout.columnSpan: 2
                text: i18n("CO₂: %1g", journeySection.co2Emission)
                visible: journeySection.co2Emission >= 0
            }
        }
    }

    ListView {
        clip: true
        model: journeySection.intermediateStops

        header: JourneySectionStopDelegate {
            stop: journeySection.departure
            isDeparture: true
        }
        delegate: JourneySectionStopDelegate {
            stop: modelData
        }
        footer: JourneySectionStopDelegate {
            stop: journeySection.arrival
            isArrival: true
        }
    }
}
