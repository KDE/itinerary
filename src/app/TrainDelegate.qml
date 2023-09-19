// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

App.TimelineDelegate {
    id: root
    headerIcon {
        source: departure.route.line.mode == Line.Unknown ? "qrc:///images/train.svg" : PublicTransport.lineIcon(departure.route.line)
        isMask: !departure.route.line.hasLogo && !departure.route.line.hasModeLogo
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

            ColumnLayout{
                spacing:0
                Layout.fillHeight: true
                Layout.fillWidth: true
                RowLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    QQC2.Label {
                        Layout.fillWidth: true
                        font.bold: true
                        text: reservationFor.departureStation.name
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
                Row {
                    Layout.fillWidth: true

                    Layout.preferredWidth: topLayout.width
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
                    Layout.fillWidth: true

                    visible: text.length > 0
                    text: Localizer.formatAddressWithContext(reservationFor.departureStation.address,
                                                             reservationFor.arrivalStation.address,
                                                             Settings.homeCountryIsoCode)
                    width: topLayout.width
                }
                Kirigami.Separator {
                    Layout.topMargin: Kirigami.Units.smallSpacing
                    Layout.bottomMargin: Kirigami.Units.smallSpacing
                    Layout.fillWidth: true
                }
            }

        }

        // TODO reserved seat

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
            ColumnLayout{
                spacing:0
                Layout.fillHeight: true
                Layout.fillWidth: true
                RowLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    QQC2.Label {
                        Layout.fillWidth: true
                        font.bold: true
                        text: reservationFor.arrivalStation.name
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
                                return i18n("");
                            }
                        }
                    }
                }

                Row {
                    Layout.fillWidth: true

                    Layout.preferredWidth: topLayout.width
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
                        text: i18nc("Arrival at the location is a day later", "(+ 1 day)")
                        color: Kirigami.Theme.disabledTextColor
                        visible: Localizer.formatDate(reservationFor, "arrivalTime") !== Localizer.formatDate(reservationFor, "departureTime")
                        Accessible.ignored: !visible
                    }
                }
                QQC2.Label {
                    Layout.fillWidth: true

                    visible: text.length > 0
                    width: topLayout.width
                    text: Localizer.formatAddressWithContext(reservationFor.arrivalStation.address,
                                                             reservationFor.departureStation.address,
                                                             Settings.homeCountryIsoCode)
                }
            }

        }


    }

    onClicked: showDetailsPage(trainDetailsPage, root.batchId)
    Accessible.name: headerLabel.text
}
