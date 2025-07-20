// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport
import org.kde.itinerary

/** Expandable intermediate stop view for timeline entries. */
Item {
    id: root

    /** The JourneySection displayed here. */
    property JourneySectionModel sectionModel
    /** Whether intermediate stops are expanded. */
    property bool expanded: false

    implicitHeight: children.filter(item => item !== stopRepeater && item !== bar)
        .reduce((currentValue, item) => item.implicitHeight + currentValue, 0)

    Layout.fillWidth: true

    Behavior on implicitHeight {
        NumberAnimation {
            duration: Kirigami.Units.shortDuration
        }
    }

    Rectangle {
        id: bar
        anchors {
            top: parent.top
            bottom: parent.bottom
        }
        width: Kirigami.Units.smallSpacing * 4
        x: Math.round((Kirigami.Units.iconSizes.smallMedium - width) / 2)

        color: root.sectionModel.journeySection.route.line.hasColor ? root.sectionModel.journeySection.route.line.color : Kirigami.Theme.textColor
    }

    Repeater {
        id: stopRepeater

        model: root.expanded ? root.sectionModel : []
        delegate: RowLayout {
            id: stopDelegate

            required property KPublicTransport.stopover stopover
            required property real progress
            required property int index

            spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

            width: parent.width
            y: index * height

            JourneySectionStopDelegateLineSegment {
                id: intermediateStopLine
                Layout.fillHeight: true
                lineColor: bar.color
                hasStop: stopDelegate.stopover.disruptionEffect !== KPublicTransport.Disruption.NoService && (stopDelegate.stopover.pickupType !== KPublicTransport.PickupDropoff.NotAllowed || stopDelegate.stopover.dropoffType !== KPublicTransport.PickupDropoff.NotAllowed)
                progress: stopDelegate.progress
            }
            QQC2.Label{
                text: stopDelegate.stopover.stopPoint.name
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.strikeout: stopDelegate.stopover.disruptionEffect === KPublicTransport.Disruption.NoService
                opacity: (stopDelegate.stopover.pickupType !== KPublicTransport.PickupDropoff.NotAllowed || stopDelegate.stopover.dropoffType !== KPublicTransport.PickupDropoff.NotAllowed) ? 1.0 : 0.5
            }
            RowLayout {
                QQC2.Label {
                    opacity: (stopDelegate.stopover.pickupType !== KPublicTransport.PickupDropoff.NotAllowed || stopDelegate.stopover.dropoffType !== KPublicTransport.PickupDropoff.NotAllowed) ? 0.8 : 0.4
                    text: Localizer.formatTime(stopDelegate.stopover, stopDelegate.stopover.scheduledDepartureTime > 0 ? "scheduledDepartureTime" : "scheduledArrivalTime")
                    font.strikeout: stopDelegate.stopover.disruptionEffect === KPublicTransport.Disruption.NoService
                }
                QQC2.Label {
                    id: stopDelayLabel
                    readonly property int delay: stopDelegate.stopover.scheduledDepartureTime > 0 ? stopDelegate.stopover.departureDelay : stopDelegate.stopover.arrivalDelay
                    text: (stopDelayLabel.delay >= 0 ? "+" : "") + stopDelayLabel.delay
                    color: (stopDelayLabel.delay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                    visible: stopDelegate.stopover.hasExpectedArrivalTime && stopDelegate.stopover.disruptionEffect !== KPublicTransport.Disruption.NoService
                    opacity: (stopDelegate.stopover.pickupType !== KPublicTransport.PickupDropoff.NotAllowed || stopDelegate.stopover.dropoffType !== KPublicTransport.PickupDropoff.NotAllowed) ? 1.0 : 0.5
                    Accessible.ignored: !visible
                }
            }
        }
    }
}
