// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

App.TimelineDelegate {
    id: root
    property bool expanded: false
    property var journeySection: root.controller.journey
    headerIcon {
        source: departure.route.line.mode == Line.Unknown ? "qrc:///images/train.svg" : PublicTransport.lineIcon(departure.route.line)
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
                        font.strikeout: departure.hasExpectedPlatform
                        text: reservationFor.departurePlatform ? i18nc("Train platform, keep short", "Pl. %1", reservationFor.departurePlatform) : ""
                        visible: text !== ""
                    }
                    QQC2.Label {
                        Layout.alignment: Qt.AlignRight
                        text: departure.hasExpectedPlatform ? i18nc("Train platform, keep short", "Pl. %1", departure.expectedPlatform) : ""
                        visible: text !== ""
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
                        text: stopRepeater.count ===1 ? i18n("1 intermediate stop"):i18n("%1 intermediate stops", stopRepeater.count)
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
                    hasStop: true
                }
                RowLayout {
                    Layout.minimumWidth: depTime.width + Kirigami.Units.largeSpacing * 3.5
                    QQC2.Label{
                        text: Localizer.formatTime(model.stopover , "scheduledDepartureTime")
                    }
                    QQC2.Label {
                        text: (model.stopover.arrivalDelay >= 0 ? "+" : "") + model.stopover.arrivalDelay
                        color: (model.stopover.arrivalDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                        visible: model.stopover.hasExpectedArrivalTime
                        Accessible.ignored: !visible
                    }
                }
                QQC2.Label{
                    text: model.stopover.stopPoint.name
                    Layout.fillWidth: true
                    elide: Text.ElideRight

                }
            }
        }


        // TODO reserved seat

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
                        font.strikeout: arrival.hasExpectedPlatform
                        text: reservationFor.arrivalPlatform ? i18nc("Train platform, keep short", "Pl. %1", reservationFor.arrivalPlatform) : ""
                        visible: text !== ""
                    }
                    QQC2.Label {
                        Layout.alignment: Qt.AlignRight
                        text: arrival.hasExpectedPlatform ? i18nc("Train platform, keep short", "Pl. %1", arrival.expectedPlatform) : ""
                        visible: text !== ""
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

    onClicked: showDetailsPage(trainDetailsPage, root.batchId)
    Accessible.name: headerLabel.text
}
