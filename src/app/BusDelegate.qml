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
        showProgress: root.controller.isCurrent
    }

    contentItem: ColumnLayout {
        spacing: 0

        TimelineDelegateDepartureLayout {
            id: departureLayout

            progress: root.expanded ? sectionModel.departureProgress : root.controller.progress
            isCanceled: root.controller.isCanceled
            reservationFor: root.reservationFor
            transportName: if (reservationFor.busName || reservationFor.busNumber ) {
                return reservationFor.busName + " " + reservationFor.busNumber
            } else {
                return i18nc("@info", "Bus")
            }

            transportIcon: departure.route.line.mode == Line.Unknown ?  ReservationHelper.defaultIconName(root.reservation) : departure.route.line.iconName
            departure: root.departure
            departureName: reservationFor.departureBusStop.name
            departurePlatform: {
                let platform = "";
                if (departure.hasExpectedPlatform) {
                    platform = departure.expectedPlatform;
                } else if (reservationFor.departurePlatform) {
                    platform = reservationFor.departurePlatform;
                }

                if (platform && root.departure.platformChanged) {
                    return i18nc("bus station platform", "Platform changed to %1", platform);
                } else if (platform) {
                    return i18nc("bus station platform", "Platform %1", platform);
                } else {
                    return "";
                }
            }
            departurePlatformChanged: root.departure.platformChanged

            departureCountry: Localizer.formatCountryWithContext(reservationFor.departureBusStop.address,
                                                                 reservationFor.arrivalBusStop.address,
                                                                 Settings.homeCountryIsoCode)

            TimelineDelegateSeatRow {
                hasSeat: root.hasSeat

                TimelineDelegateSeatRowLabel {
                    text: i18nc("bus seat", "Seat: <b>%1</b>", root.seatString)
                }
            }

            TimelineDelegateIntermediateStopsButton {
                sectionModel: sectionModel
                expanded: root.expanded
                onToggled: root.expanded = !root.expanded
            }
        }

        TimelineDelegateIntermediateStopsView {
            sectionModel: sectionModel
            expanded: root.expanded
            isCanceled: root.controller.isCanceled
        }

        TimelineDelegateArrivalLayout {
            reservationFor: root.reservationFor
            arrival: root.arrival
            arrivalName: reservationFor.arrivalBusStop.name
            isCanceled: root.controller.isCanceled
            arrivalCountry: Localizer.formatCountryWithContext(reservationFor.arrivalBusStop.address,
                                                                 reservationFor.departureBusStop.address,
                                                                 Settings.homeCountryIsoCode)
            arrivalPlatform: {
                let platform = "";
                if (arrival.hasExpectedPlatform) {
                    platform = arrival.expectedPlatform;
                } else if (reservationFor.arrivalPlatform) {
                    platform = reservationFor.arrivalPlatform;
                }

                if (platform && root.arrival.platformChanged) {
                    return i18nc("bus station platform", "Platform changed to %1", platform);
                } else if (platform) {
                    return i18nc("bus station platform", "Platform %1", platform);
                } else {
                    return "";
                }
            }
            arrivalPlatformChanged: root.arrival.platformChanged
            progress: sectionModel.arrived ? 1 : 0
        }
    }

    Accessible.name: {
        if (reservationFor.busName || reservationFor.busNumber ) {
            return reservationFor.busName + " " + reservationFor.busNumber
        }
        return i18nc("from location 1 to location 2", "%1 to %2", reservationFor.departureBusStop.name, reservationFor.arrivalBusStop.name);
    }
}
