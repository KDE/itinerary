/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

App.TimelineDelegate {
    id: root
    headerIconSource: departure.route.line.mode == Line.Unknown ? "qrc:///images/bus.svg" : PublicTransport.lineModeIcon(departure.route.line.mode)
    headerItem: RowLayout {
        QQC2.Label {
            id: headerLabel
            text: {
                if (reservationFor.busName || reservationFor.busNumber ) {
                    return reservationFor.busName + " " + reservationFor.busNumber
                }
                return i18n("%1 to %2", reservationFor.departureBusStop.name, reservationFor.arrivalBusStop.name);
            }
            color: root.headerTextColor
            Layout.fillWidth: true
            Accessible.ignored: true
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
            ColumnLayout {
                spacing: 0
                JourneySectionStopDelegateLineSegment {

                    Layout.fillHeight: true
                    lineColor: departure.route.line.hasColor ? departure.route.line.color : Kirigami.Theme.textColor
                    isDeparture: true
                }
                JourneySectionStopDelegateLineSegment {

                    Layout.fillHeight: true
                    lineColor: departure.route.line.hasColor ? departure.route.line.color : Kirigami.Theme.textColor
                    isDeparture: false
                    hasStop: false
                }
            }

            ColumnLayout {
                spacing: 0
                Layout.fillHeight: true
                Layout.fillWidth: true

                QQC2.Label {
                    Layout.fillWidth: true
                    font.bold: true
                    text: reservationFor.departureBusStop.name
                }
                Row {
                    Layout.fillWidth: true
                    width: topLayout.width
                    spacing: Kirigami.Units.smallSpacing
                    QQC2.Label {
                        text: i18n("Departure: %1", Localizer.formatTime(reservationFor, "departureTime"))
                        color: Kirigami.Theme.textColor
                        wrapMode: Text.WordWrap
                        visible: reservationFor.arrivalTime > 0
                    }
                    QQC2.Label {
                        text: (arrival.arrivalDelay >= 0 ? "+" : "") + arrival.arrivalDelay
                        color: (arrival.arrivalDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                        visible: arrival.hasExpectedArrivalTime
                        Accessible.ignored: !visible
                    }
                }
                QQC2.Label {
                    visible: text.length > 0
                    Layout.fillWidth: true
                    text: Localizer.formatAddressWithContext(reservationFor.departureBusStop.address,
                                                             reservationFor.arrivalBusStop.address,
                                                             Settings.homeCountryIsoCode)
                }
                Kirigami.Separator {
                    Layout.topMargin: Kirigami.Units.smallSpacing
                    Layout.bottomMargin: Kirigami.Units.smallSpacing
                    Layout.fillWidth: true
                }
            }

        }

        RowLayout {
            width: parent.width
            ColumnLayout {
                spacing: 0
                JourneySectionStopDelegateLineSegment {

                    Layout.fillHeight: true
                    lineColor: departure.route.line.hasColor ? departure.route.line.color : Kirigami.Theme.textColor
                    isDeparture: false
                    hasStop: false
                }
                JourneySectionStopDelegateLineSegment {

                    Layout.fillHeight: true
                    lineColor: departure.route.line.hasColor ? departure.route.line.color : Kirigami.Theme.textColor
                    isArrival: true
                }
            }
            ColumnLayout {
                spacing: 0
                Layout.fillHeight: true
                Layout.fillWidth: true

                QQC2.Label {
                    Layout.fillWidth: true
                    font.bold: true
                    text: reservationFor.arrivalBusStop.name
                }
                Row {
                    Layout.fillWidth: true

                    spacing: Kirigami.Units.smallSpacing
                    QQC2.Label {
                        text: i18n("Arrival: %1", Localizer.formatTime(reservationFor, "arrivalTime"))
                        color: Kirigami.Theme.textColor
                        wrapMode: Text.WordWrap
                        visible: reservationFor.arrivalTime > 0
                    }
                    QQC2.Label {
                        text: (arrival.arrivalDelay >= 0 ? "+" : "") + arrival.arrivalDelay
                        color: (arrival.arrivalDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                        visible: arrival.hasExpectedArrivalTime
                        Accessible.ignored: !visible
                    }
                    QQC2.Label {
                        text: i18n("(+ 1 day)")
                        color: Kirigami.Theme.disabledTextColor
                        visible: Localizer.formatDate(reservationFor, "arrivalTime") !== Localizer.formatDate(reservationFor, "departureTime")
                        Accessible.ignored: !visible
                    }
                }
                QQC2.Label {
                    visible: text.length > 0
                    width: topLayout.width
                    text: Localizer.formatAddressWithContext(reservationFor.arrivalBusStop.address,
                                                             reservationFor.departureBusStop.address,
                                                             Settings.homeCountryIsoCode)
                }
            }

        }


    }
    onClicked: showDetailsPage(busDetailsPage, root.batchId)
    Accessible.name: headerLabel.text
}
