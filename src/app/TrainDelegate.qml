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

            progress: root.expanded ? sectionModel.departureProgress : root.controller.progress
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
            departureCountry: Localizer.formatCountryWithContext(reservationFor.departureStation.address,
                                                         reservationFor.arrivalStation.address,
                                                         Settings.homeCountryIsoCode)

            TimelineDelegateSeatRow {
                hasSeat: root.hasSeat

                TimelineDelegateSeatRowLabel {
                    text: i18nc("train coach", "Coach: <b>%1</b>", root.seatSectionString)
                }
                Kirigami.Separator {
                    Layout.fillHeight: true
                }
                TimelineDelegateSeatRowLabel {
                    text: i18nc("train seat", "Seat: <b>%1</b>", root.seatString)
                }
                Kirigami.Separator {
                    Layout.fillHeight: true
                }
                TimelineDelegateSeatRowLabel {
                    text: {
                        const s = root.reservation?.reservedTicket?.ticketedSeat?.seatingType;
                        return i18nc("train class", "Class: <b>%1</b>", s || "-");
                    }
                    lowPriority: true
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
            progress: sectionModel.arrived ? 1 : 0
        }
    }

    Accessible.name: if (reservationFor.trainName || reservationFor.trainNumber) {
        return reservationFor.trainName + " " + reservationFor.trainNumber;
    } else {
        return i18nc("from location 1 to location 2", "%1 to %2", reservationFor.departureStation.name, reservationFor.arrivalStation.name);
    }
}
