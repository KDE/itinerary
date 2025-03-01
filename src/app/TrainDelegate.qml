// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport
import org.kde.itinerary

TimelineDelegate {
    id: root

    property bool expanded: false
    property var journeySection: root.controller.journey

    JourneySectionModel {
        id: sectionModel
        journeySection: root.journeySection
        showProgress: root.controller.isCurrent
    }

    contentItem: ColumnLayout {
        spacing: 0

        TimelineDelegateDepartureLayout {
            id: departureLayout

            progress: root.controller.progress
            reservationFor: root.reservationFor
            transportName: root.reservationFor.trainName + " " + root.reservationFor.trainNumber
            transportIcon: departure.route.line.mode == Line.Unknown ? ReservationHelper.defaultIconName(root.reservation) : departure.route.line.iconName
            departure: root.departure
            departureName: root.reservationFor.departureStation.name
            departurePlatform: {
                let platform = "";
                if (departure.hasExpectedPlatform) {
                    platform = departure.expectedPlatform;
                } else if (reservationFor.departurePlatform) {
                    platform = reservationFor.departurePlatform;
                }

                if (platform && root.departure.platformChanged) {
                    return i18nc("train station platform", "Platform changed to %1", platform);
                } else if (platform) {
                    return i18nc("train station platform", "Platform %1", platform);
                } else {
                    return "";
                }
            }
            departurePlatformChanged: root.departure.platformChanged
            Component.onCompleted: {
                progress = root.controller.progress;
            }

            departureCountry: Localizer.formatCountryWithContext(reservationFor.departureStation.address,
                                                         reservationFor.arrivalStation.address,
                                                         Settings.homeCountryIsoCode)
        }

        TimelineDelegateSeatRow {
            route: root.departure.route
            hasSeat: root.hasSeat

            Component.onCompleted: progress = root.controller.progress

            TimelineDelegateSeatRowLabel {
                text: i18nc("train coach", "Coach: <b>%1</b>", root.reservation?.reservedTicket?.ticketedSeat?.seatSection || "-")
            }
            Kirigami.Separator {
                Layout.fillHeight: true
            }
            TimelineDelegateSeatRowLabel {
                text: i18nc("train seat", "Seat: <b>%1</b>", root.seatString())
            }
            Kirigami.Separator {
                Layout.fillHeight: true
            }
            TimelineDelegateSeatRowLabel {
                text: {
                    const s = root.reservation?.reservedTicket?.ticketedSeat?.seatingType;
                    return i18nc("train class", "Class: <b>%1</b>", s !== "" ? s : "-");
                }
                lowPriority: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

            JourneySectionStopDelegateLineSegment {
                Layout.fillHeight: true
                lineColor: departure.route.line.hasColor ? departure.route.line.color : Kirigami.Theme.textColor
                isDeparture: false
                hasStop: false

                Component.onCompleted: {
                    leadingProgress = controller.progress > 0 ? 1.0 : 0;
                    trailingProgress = controller.progress > 0 ? 1.0 : 0;
                }
            }
            QQC2.ToolButton {
                visible: sectionModel.sectionCount !== 0
                Layout.fillWidth: true
                onClicked: expanded = !expanded
                contentItem: RowLayout {
                    spacing: 0
                    Kirigami.Icon {
                        source: expanded? "arrow-up" : "arrow-down"
                        implicitHeight: Kirigami.Units.largeSpacing*2
                        color: Kirigami.Theme.disabledTextColor
                    }
                    QQC2.Label {
                        text: i18np("1 intermediate stop (%2)", "%1 intermediate stops (%2)", sectionModel.sectionCount, Localizer.formatDuration(root.journeySection.duration))
                        elide: Text.ElideRight
                        color: Kirigami.Theme.disabledTextColor
                        Layout.rightMargin: Kirigami.Units.largeSpacing
                        Layout.fillWidth: true
                    }
                }
            }
            QQC2.Label {
                visible: sectionModel.sectionCount === 0 && root.journeySection.duration > 0
                text: i18n("0 intermediate stop (%1)", Localizer.formatDuration(root.journeySection.duration))
                elide: Text.ElideRight
                color: Kirigami.Theme.disabledTextColor
                Layout.fillWidth: true
            }
        }

        Item {
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

                color: departure.route.line.hasColor ? departure.route.line.color : Kirigami.Theme.textColor
            }

            Repeater {
                id: stopRepeater

                model: root.expanded ? sectionModel : []
                delegate: RowLayout {
                    id: stopDelegate

                    spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

                    width: parent.width
                    y: index * height

                    JourneySectionStopDelegateLineSegment {
                        id: intermediateStopLine
                        Layout.fillHeight: true
                        lineColor: departure.route.line.hasColor ? departure.route.line.color : Kirigami.Theme.textColor
                        hasStop: model.stopover.disruptionEffect !== Disruption.NoService

                        leadingProgress: model.leadingProgress
                        trailingProgress: model.trailingProgress
                        Binding {
                            target: model
                            property: "leadingLength"
                            value: intermediateStopLine.leadingLineLength
                        }
                        Binding {
                            target: model
                            property: "trailingLength"
                            value: intermediateStopLine.trailingLineLength
                        }
                    }
                    QQC2.Label{
                        text: model.stopover.stopPoint.name
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        font.strikeout: model.stopover.disruptionEffect === Disruption.NoService
                    }
                    RowLayout {
                        QQC2.Label {
                            opacity: 0.8
                            text: Localizer.formatTime(model.stopover , model.stopover.scheduledDepartureTime > 0 ? "scheduledDepartureTime" : "scheduledArrivalTime")
                            font.strikeout: model.stopover.disruptionEffect === Disruption.NoService
                        }
                        QQC2.Label {
                            id: stopDelayLabel
                            readonly property int delay: model.stopover.scheduledDepartureTime > 0 ? model.stopover.departureDelay : model.stopover.arrivalDelay
                            text: (stopDelayLabel.delay >= 0 ? "+" : "") + stopDelayLabel.delay
                            color: (stopDelayLabel.delay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                            visible: model.stopover.hasExpectedArrivalTime && model.stopover.disruptionEffect !== Disruption.NoService
                            Accessible.ignored: !visible
                        }
                    }
                }
            }
        }

        TimelineDelegateArrivalLayout {
            reservationFor: root.reservationFor
            arrival: root.arrival
            departure: root.departure
            arrivalName: reservationFor.arrivalStation.name
            arrivalCountry: Localizer.formatCountryWithContext(reservationFor.arrivalStation.address,
                                                         reservationFor.departureStation.address,
                                                         Settings.homeCountryIsoCode)
            arrivalPlatform: {
                let platform = "";
                if (arrival.hasExpectedPlatform) {
                    platform = arrival.expectedPlatform;
                } else if (reservationFor.arrivalPlatform) {
                    platform = reservationFor.arrivalPlatform;
                }

                if (platform && root.arrival.platformChanged) {
                    return i18nc("train station platform", "Platform changed to %1", platform);
                } else if (platform) {
                    return i18nc("train station platform", "Platform %1", platform);
                } else {
                    return "";
                }
            }
            arrivalPlatformChanged: root.arrival.platformChanged

            Component.onCompleted: {
                progress = root.controller.progress;
            }
        }
    }

    Accessible.name: if (reservationFor.trainName || reservationFor.trainNumber) {
        return reservationFor.trainName + " " + reservationFor.trainNumber;
    } else {
        return i18nc("from location 1 to location 2", "%1 to %2", reservationFor.departureStation.name, reservationFor.arrivalStation.name);
    }
}
