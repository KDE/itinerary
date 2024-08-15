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
    headerIcon {
        source: departure.route.line.mode == Line.Unknown ? ReservationHelper.defaultIconName(root.reservation) : departure.route.line.iconName
        isMask: !departure.route.line.hasLogo && !departure.route.line.hasModeLogo
    }
    Item {
        JourneySectionModel {
            id: sectionModel
            journeySection: root.journeySection
        }
    }

    headerItem: RowLayout {
        QQC2.Label {
            id: headerLabel
            text: {
                if (reservationFor.trainName || reservationFor.trainNumber) {
                    return reservationFor.trainName + " " + reservationFor.trainNumber
                }
                return i18n("%1 to %2", reservationFor.departureStation.name, reservationFor.arrivalStation.name);
            }
            color: root.headerTextColor
            elide: Text.ElideRight
            Accessible.ignored: true
        }
        QQC2.Label {
            text: departure.route.direction? "→ " + departure.route.direction : ""
            color: root.headerTextColor
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: Localizer.formatTime(reservationFor, "departureTime")
            color: root.headerTextColor
        }
        QQC2.Label {
            text: (departure.departureDelay >= 0 ? "+" : "") + departure.departureDelay
            color: (departure.departureDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
            visible: departure.hasExpectedDepartureTime
            Accessible.ignored: !visible
        }
    }

    contentItem: Column {
        id: topLayout

        RowLayout {
            width: parent.width
            ColumnLayout{
                spacing: 0
                JourneySectionStopDelegateLineSegment {
                    Layout.fillHeight: true
                    lineColor: departure.route.line.hasColor ? departure.route.line.color : Kirigami.Theme.textColor
                    isDeparture: true
                }
                JourneySectionStopDelegateLineSegment {
                    visible: departureCountryLayout.visible
                    Layout.fillHeight: true
                    lineColor: departure.route.line.hasColor ? departure.route.line.color : Kirigami.Theme.textColor
                    hasStop: false
                }
            }

            ColumnLayout{
                Layout.bottomMargin:  Kirigami.Units.largeSpacing

                spacing:0
                Layout.fillHeight: true
                Layout.fillWidth: true
                RowLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    RowLayout {
                        Layout.minimumWidth: depTime.width + Kirigami.Units.largeSpacing * 3.5
                        QQC2.Label {
                            id: depTime
                            text: Localizer.formatTime(reservationFor, "departureTime")
                        }
                        QQC2.Label {
                            text: (departure.departureDelay >= 0 ? "+" : "") + departure.departureDelay
                            color: (departure.departureDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                            visible: departure.hasExpectedDepartureTime
                            Accessible.ignored: !visible
                        }
                    }
                    QQC2.Label {
                        Layout.fillWidth: true
                        font.bold: true
                        text: reservationFor.departureStation.name
                        elide: Text.ElideRight
                    }
                    QQC2.Label {
                        Layout.alignment: Qt.AlignRight
                        text: {
                            let platform = "";
                            if (departure.hasExpectedPlatform) {
                                platform = departure.expectedPlatform;
                            } else if (reservationFor.departurePlatform) {
                                platform = reservationFor.departurePlatform;
                            }

                            if (platform) {
                                return i18n("Pl. %1", platform);
                            } else {
                                return "";
                            }
                        }
                    }
                }
                RowLayout{
                    id: departureCountryLayout
                    visible: departureCountryLabel.text.length > 0
                    Item{
                        Layout.minimumWidth: depTime.width + Kirigami.Units.largeSpacing * 3.5
                    }
                    QQC2.Label {
                        id: departureCountryLabel
                        Layout.fillWidth: true
                        text: Localizer.formatAddressWithContext(reservationFor.departureStation.address,
                                                                 reservationFor.arrivalStation.address,
                                                                 Settings.homeCountryIsoCode)
                        width: topLayout.width
                    }
                }
            }
        }

        RowLayout {
            visible: root.hasSeat
            width: parent.width
            JourneySectionStopDelegateLineSegment {
                Layout.fillHeight: true
                lineColor: departure.route.line.hasColor ? departure.route.line.color : Kirigami.Theme.textColor
                isDeparture: false
                hasStop: false
            }
            TimelineDelegateSeatRow {
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
        }

        RowLayout {
            width: parent.width
            JourneySectionStopDelegateLineSegment {
                Layout.fillHeight: true
                lineColor: departure.route.line.hasColor ? departure.route.line.color : Kirigami.Theme.textColor
                isDeparture: false
                hasStop: false
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
                        text: i18np("1 intermediate stop", "%1 intermediate stops", stopRepeater.count)
                        color: Kirigami.Theme.disabledTextColor
                        Layout.rightMargin: Kirigami.Units.largeSpacing
                    }
                    Kirigami.Separator {
                        Layout.topMargin: Kirigami.Units.smallSpacing
                        Layout.bottomMargin: Kirigami.Units.smallSpacing
                        Layout.fillWidth: true
                    }
                }
            }
            Kirigami.Separator {
                visible: stopRepeater.count === 0
                Layout.topMargin: Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing
                Layout.fillWidth: true
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
                onHiddenChanged:
                    if (!hidden) {
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
                width: parent.width
                JourneySectionStopDelegateLineSegment {

                    Layout.fillHeight: true
                    lineColor: departure.route.line.hasColor ? departure.route.line.color : Kirigami.Theme.textColor
                    hasStop: model.stopover.disruptionEffect !== Disruption.NoService
                }
                RowLayout {
                    Layout.minimumWidth: depTime.width + Kirigami.Units.largeSpacing * 3.5
                    QQC2.Label{
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
                QQC2.Label{
                    text: model.stopover.stopPoint.name
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    font.strikeout: model.stopover.disruptionEffect === Disruption.NoService
                }
            }
        }

        RowLayout {
            width: parent.width
            ColumnLayout {
                spacing: 0
                JourneySectionStopDelegateLineSegment {
                    visible: arrivalCountryLayout.visible
                    Layout.fillHeight: true
                    lineColor: departure.route.line.hasColor ? departure.route.line.color : Kirigami.Theme.textColor
                    hasStop: false
                }
                JourneySectionStopDelegateLineSegment {
                    Layout.fillHeight: true
                    lineColor: departure.route.line.hasColor ? departure.route.line.color : Kirigami.Theme.textColor
                    isArrival: true
                }
            }
            ColumnLayout{
                Layout.topMargin:  Kirigami.Units.largeSpacing

                spacing:0
                Layout.fillHeight: true
                Layout.fillWidth: true
                RowLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    RowLayout {
                        Layout.minimumWidth: depTime.width + Kirigami.Units.largeSpacing * 3.5
                        QQC2.Label {
                            text: Localizer.formatTime(reservationFor, "arrivalTime")
                        }
                        QQC2.Label {
                            text: (arrival.arrivalDelay >= 0 ? "+" : "") + arrival.arrivalDelay
                            color: (arrival.arrivalDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                            visible: arrival.hasExpectedArrivalTime
                            Accessible.ignored: !visible
                        }
                    }
                    QQC2.Label {
                        Layout.fillWidth: true
                        font.bold: true
                        text: reservationFor.arrivalStation.name
                        elide: Text.ElideRight
                    }
                    QQC2.Label {
                        Layout.alignment: Qt.AlignRight
                        text: {
                            let platform = "";
                            if (arrival.hasExpectedPlatform) {
                                platform = arrival.expectedPlatform;
                            } else if (reservationFor.arrivalPlatform) {
                                platform = reservationFor.arrivalPlatform;
                            }

                            if (platform) {
                                return i18n("Pl. %1", platform);
                            } else {
                                return "";
                            }
                        }
                    }
                }
                RowLayout {
                    id: arrivalCountryLayout
                    visible: arrivalCountryLabel.text.length > 0
                    Item{
                        Layout.minimumWidth: depTime.width + Kirigami.Units.largeSpacing * 3.5
                    }
                    QQC2.Label {
                        id: arrivalCountryLabel
                        Layout.fillWidth: true
                        width: topLayout.width
                        text: Localizer.formatAddressWithContext(reservationFor.arrivalStation.address,
                                                                 reservationFor.departureStation.address,
                                                                 Settings.homeCountryIsoCode)
                    }
                }
            }
        }
    }

    Accessible.name: headerLabel.text
}
