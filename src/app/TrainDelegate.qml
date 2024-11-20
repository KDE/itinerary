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
        showProgress: true
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

                if (platform) {
                    return i18n("Platform %1", platform);
                } else {
                    return "";
                }
            }
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
                visible: stopRepeater.count !== 0
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
                        text: i18np("1 intermediate stop (%2)", "%1 intermediate stops (%2)", stopRepeater.count, Localizer.formatDurationCustom(root.journeySection.duration))
                        elide: Text.ElideRight
                        color: Kirigami.Theme.disabledTextColor
                        Layout.rightMargin: Kirigami.Units.largeSpacing
                        Layout.fillWidth: true
                    }
                }
            }
        }

        Repeater {
            id: stopRepeater
            model: sectionModel
            delegate: RowLayout {
                id: stopDelegate

                property bool hidden: !expanded

                clip: true
                visible: false
                spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                Layout.fillWidth: true
                onHiddenChanged: if (!hidden) {
                    visible = true
                    showAnimation.running=true
                } else {
                    hideAnimation.running = true
                }
                PropertyAnimation { id: showAnimation;
                                    target: stopDelegate;
                                    property: "height";
                                    from: 0;
                                    to: stopDelegate.implicitHeight;
                                    duration: 200
                                    easing.type: Easing.InOutCubic
                }

                PropertyAnimation { id: hideAnimation;
                                    target: stopDelegate;
                                    property: "height";
                                    from: stopDelegate.implicitHeight;
                                    to: 0;
                                    duration: 200
                                    onFinished: stopDelegate.visible = false
                                    easing.type: Easing.InOutCubic

                }
                JourneySectionStopDelegateLineSegment {

                    Layout.fillHeight: true
                    lineColor: departure.route.line.hasColor ? departure.route.line.color : Kirigami.Theme.textColor
                    hasStop: model.stopover.disruptionEffect !== Disruption.NoService

                    leadingProgress: root.progress > 0 && model.stopoverPassed ? 1.0 : 0
                    trailingProgress: root.progress > 0 && model.stopoverPassed ? 1.0 : 0
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

                if (platform) {
                    return i18n("Platform %1", platform);
                } else {
                    return "";
                }
            }

            Component.onCompleted: {
                progress = root.controller.progress;
            }
        }
    }

    Accessible.name: if (reservationFor.trainName || reservationFor.trainNumber) {
        return reservationFor.trainName + " " + reservationFor.trainNumber;
    } else {
        return i18n("%1 to %2", reservationFor.departureStation.name, reservationFor.arrivalStation.name);
    }
}
