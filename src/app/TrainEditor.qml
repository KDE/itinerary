/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

App.EditorPage {
    id: root
    title: i18n("Edit Train Reservation")

    property var departureStation: reservationFor.departureStation
    property var departureTime: Util.dateTimeStripTimezone(reservationFor, "departureTime")
    property var arrivalStation: reservationFor.arrivalStation
    property var arrivalTime: Util.dateTimeStripTimezone(reservationFor, "arrivalTime")

    actions.contextualActions: [
        Kirigami.Action {
            text: i18n("Board later")
            icon.name: "arrow-left"
            enabled: root.controller.journey && root.controller.journey.intermediateStops.length > 0 // TODO also check for preceding layovers
            onTriggered: boardSheet.open();
        },
        Kirigami.Action {
            text: i18n("Alight earlier")
            icon.name: "arrow-right"
            enabled: root.controller.journey && root.controller.journey.intermediateStops.length > 0 // TODO also check for subsequent layovers
            onTriggered: alightSheet.open();
        }
    ]

    function save(resId, reservation) {
        var trip = reservation.reservationFor;
        trip.departureStation = root.departureStation;
        trip = Util.setDateTimePreserveTimezone(trip, "departureTime", root.departureTime);
        trip.departurePlatform = departurePlatform.text;
        trip.arrivalStation = root.arrivalStation;
        trip = Util.setDateTimePreserveTimezone(trip, "arrivalTime", root.arrivalTime);
        trip.arrivalPlatform = arrivalPlatform.text;

        var seat = reservation.reservedTicket.ticketedSeat;
        seat.seatSection = coachEdit.text;
        seat.seatNumber = seatEdit.text;
        seat.seatingType = classEdit.text;

        var ticket = reservation.reservedTicket;
        ticket.ticketedSeat = seat;

        var newRes = reservation;
        newRes.reservationFor = trip;
        newRes.reservedTicket = ticket;

        ReservationManager.updateReservation(resId, newRes);
    }

    Component {
        id: intermediateStopDelegate
        Kirigami.BasicListItem {
            text: Localizer.formatTime(modelData, "scheduledDepartureTime") + " " + modelData.stopPoint.name
            enabled: modelData.disruptionEffect != Disruption.NoService
        }
    }

    Kirigami.OverlaySheet {
        id: boardSheet
        header: Kirigami.Heading {
            text: i18n("Board Later")
        }

        ListView {
            id: boardStopSelector
            model: root.controller.journey.intermediateStops
            delegate: intermediateStopDelegate
        }

        footer: QQC2.Button {
            text: i18n("Change departure station")
            enabled: boardStopSelector.currentIndex >= 0
            onClicked: {
                departureStation = PublicTransport.trainStationFromLocation(root.controller.journey.intermediateStops[boardStopSelector.currentIndex].stopPoint)
                departureTime = Util.dateTimeStripTimezone(root.controller.journey.intermediateStops[alightStopSelector.currentIndex], "scheduledDepartureTime");
                boardSheet.close();
            }
        }
    }
    Kirigami.OverlaySheet {
        id: alightSheet
        header: Kirigami.Heading {
            text: i18n("Alight Earlier")
        }

        ListView {
            id: alightStopSelector
            model: root.controller.journey.intermediateStops
            delegate: intermediateStopDelegate
        }

        footer: QQC2.Button {
            text: i18n("Change arrival station")
            enabled: stopSelector.currentIndex >= 0
            onClicked: {
                arrivalStation = PublicTransport.trainStationFromLocation(root.controller.journey.intermediateStops[alightStopSelector.currentIndex].stopPoint);
                arrivalTime = Util.dateTimeStripTimezone(root.controller.journey.intermediateStops[alightStopSelector.currentIndex], "scheduledArrivalTime");
                if (!arrivalTime) {
                    arrivalTime = Util.dateTimeStripTimezone(root.controller.journey.intermediateStops[alightStopSelector.currentIndex], "scheduledDepartureTime");
                }
                alightSheet.close();
            }
        }
    }

    Kirigami.FormLayout {
        width: parent.width

        // departure data
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Departure")
        }

        // TODO time
        QQC2.Label {
            Kirigami.FormData.label: i18nc("train station", "Station:")
            text: root.departureStation.name
        }
        QQC2.TextField {
            id: departurePlatform
            Kirigami.FormData.label: i18n("Platform:")
            text: reservationFor.departurePlatform
        }

        // arrival data
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Arrival")
        }

        // TODO time
        QQC2.Label {
            Kirigami.FormData.label: i18nc("train station", "Station:")
            text: root.arrivalStation.name
        }
        QQC2.TextField {
            id: arrivalPlatform
            Kirigami.FormData.label: i18n("Platform:")
            text: reservationFor.arrivalPlatform
        }

        // TODO the below is per reservation, not per batch, so add a selector for that!
        // seat reservation
        Kirigami.Separator {
            Kirigami.FormData.label: i18n("Seat")
            Kirigami.FormData.isSection: true
        }
        QQC2.TextField {
            id: coachEdit
            Kirigami.FormData.label: i18n("Coach:")
            text: reservation.reservedTicket.ticketedSeat.seatSection
        }
        QQC2.TextField {
            id: seatEdit
            Kirigami.FormData.label: i18n("Seat:")
            text: reservation.reservedTicket.ticketedSeat.seatNumber
        }
        QQC2.TextField {
            id: classEdit
            Kirigami.FormData.label: i18n("Class:")
            text: reservation.reservedTicket.ticketedSeat.seatingType
        }

        // booking details
        Kirigami.Separator {
            Kirigami.FormData.label: i18n("Booking")
            Kirigami.FormData.isSection: true
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Reference:")
            text: reservation.reservationNumber
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Under name:")
            text: reservation.underName.name
        }
    }
}
