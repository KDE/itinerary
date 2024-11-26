/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
            transportName: if (reservationFor.busName || reservationFor.busNumber ) {
                return reservationFor.busName + " " + reservationFor.busNumber
            } else {
                return i18nc("@info", "Bus")
            }

            transportIcon: departure.route.line.mode == Line.Unknown ?  ReservationHelper.defaultIconName(root.reservation) : departure.route.line.iconName
            departure: root.departure
            departureName: reservationFor.departureBusStop.name
            Component.onCompleted: {
                progress = root.controller.progress;
            }

            departureCountry: Localizer.formatCountryWithContext(reservationFor.departureBusStop.address,
                                                                 reservationFor.arrivalBusStop.address,
                                                                 Settings.homeCountryIsoCode)
        }

        TimelineDelegateSeatRow {
            route: root.departure.route
            hasSeat: root.hasSeat

            Component.onCompleted: progress = root.controller.progress
            TimelineDelegateSeatRowLabel {
                text: i18nc("bus seat", "Seat: <b>%1</b>", root.seatString())
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
            QQC2.Label {
                visible: stopRepeater.count === 0
                text: i18n("0 intermediate stop (%1)", Localizer.formatDurationCustom(root.journeySection.duration))
                elide: Text.ElideRight
                color: Kirigami.Theme.disabledTextColor
                Layout.fillWidth: true
            }
        }

        Repeater {
            id: stopRepeater
            model: sectionModel
            delegate: RowLayout {
                id: stopDelegate

                property bool hidden: !expanded

                width: parent.width
                clip: true
                visible: false
                spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
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
                }
                QQC2.Label{
                    text: model.stopover.stopPoint.name
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    font.strikeout: model.stopover.disruptionEffect === Disruption.NoService
                }
                RowLayout {
                    Layout.minimumWidth: depTime.width + Kirigami.Units.largeSpacing * 3.5
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
            arrivalName: reservationFor.arrivalBusStop.name
            arrivalCountry: Localizer.formatCountryWithContext(reservationFor.arrivalBusStop.address,
                                                                 reservationFor.departureBusStop.address,
                                                                 Settings.homeCountryIsoCode)
            Component.onCompleted: progress = root.controller.progress;
        }
    }

    Accessible.name: {
        if (reservationFor.busName || reservationFor.busNumber ) {
            return reservationFor.busName + " " + reservationFor.busNumber
        }
        return i18n("%1 to %2", reservationFor.departureBusStop.name, reservationFor.arrivalBusStop.name);
    }
}
