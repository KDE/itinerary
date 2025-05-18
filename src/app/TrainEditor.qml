/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitinerary
import org.kde.kpublictransport
import org.kde.itinerary

EditorPage {
    id: root
    title: i18n("Edit Train Reservation")

    property var departureStation: reservationFor.departureStation
    property var departureTime: Util.dateTimeStripTimezone(reservationFor, "departureTime")
    property var arrivalStation: reservationFor.arrivalStation
    property var arrivalTime: Util.dateTimeStripTimezone(reservationFor, "arrivalTime")

    function apply(reservation) {
        var trip = reservation.reservationFor;
        trip.departureStation = root.departureStation;
        trip = Util.setDateTimePreserveTimezone(trip, "departureTime", root.departureTime);
        trip.departurePlatform = departurePlatform.text;
        trip.arrivalStation = root.arrivalStation;
        trip = Util.setDateTimePreserveTimezone(trip, "arrivalTime", root.arrivalTime);
        trip.arrivalPlatform = arrivalPlatform.text;

        let ticket = reservation.reservedTicket ?? Factory.makeTicket();
        ticketTokenEdit.apply(ticket);
        let seat = ticket.ticketedSeat;
        seat.seatSection = coachEdit.text;
        seat.seatNumber = seatEdit.text;
        seat.seatingType = classEdit.text;
        ticket.ticketedSeat = seat;

        var newRes = reservation;
        newRes.reservationFor = trip;
        newRes.reservedTicket = ticket;
        programMembershipEdit.apply(newRes);
        bookingEdit.apply(newRes);
        return newRes;
    }

    IntermediateStopSelector {
        id: boardSheet
        title: i18nc("train departure", "Change Departure Station")
        model: [root.controller.trip.departure].concat(root.controller.trip.intermediateStops).concat([root.controller.trip.arrival])
        action: Kirigami.Action {
            text: i18nc("train departure", "Change departure station")
            onTriggered: {
                departureStation = PublicTransport.trainStationFromLocation(root.controller.trip.stopover(boardSheet.currentIndex).stopPoint)
                departureTime = Util.dateTimeStripTimezone(root.controller.trip.stopover(boardSheet.currentIndex), "scheduledDepartureTime");
                boardSheet.close();
            }
        }
    }
    IntermediateStopSelector {
        id: alightSheet
        title: i18nc("train arrival", "Change Arrival Station")
        model: [root.controller.trip.departure].concat(root.controller.trip.intermediateStops).concat([root.controller.trip.arrival])
        action: Kirigami.Action {
            text: i18nc("train arrival", "Change arrival station")
            onTriggered: {
                arrivalStation = PublicTransport.trainStationFromLocation(root.controller.trip.stopover(alightSheet.currentIndex).stopPoint);
                arrivalTime = Util.dateTimeStripTimezone(root.controller.trip.stopover(alightSheet.currentIndex), "scheduledArrivalTime");
                if (!arrivalTime) {
                    arrivalTime = Util.dateTimeStripTimezone(root.controller.trip.stopover(alightSheet.currentIndex), "scheduledDepartureTime");
                }
                alightSheet.close();
            }
        }
    }

    ColumnLayout {
        spacing: 0

        CardPageTitle {
            emojiIcon: "🚅"
            text: if (reservationFor.trainNumber || reservationFor.trainName) {
                return reservationFor.trainName + " " + reservationFor.trainNumber;
            } else {
                return i18n("Train")
            }
        }

        FormCard.FormHeader {
            title: i18nc("train departure", "Departure")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                text: i18nc("train station", "Station")
                description: root.departureStation.name
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormTextFieldDelegate {
                id: departurePlatform
                label: i18nc("train platform", "Platform")
                text: reservationFor.departurePlatform
            }
            FormCard.FormDelegateSeparator { above: boardLater }
            FormCard.FormButtonDelegate {
                id: boardLater

                text: i18nc("train departure", "Change Departure Station")
                icon.name: "document-edit"
                visible: root.controller.trip && root.controller.trip.intermediateStops.length > 0 // TODO also check for preceding layovers
                onClicked: boardSheet.open();
            }
        }

        FormCard.FormHeader {
            title: i18nc("train arrival", "Arrival")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                text: i18nc("train station", "Station")
                description: root.arrivalStation.name
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormTextFieldDelegate {
                id: arrivalPlatform
                label: i18nc("train platform", "Platform")
                text: reservationFor.arrivalPlatform
            }
            FormCard.FormDelegateSeparator { above: alignEarlier }
            FormCard.FormButtonDelegate {
                id: alignEarlier
                text: i18nc("train arrival", "Change Arrival Station")
                icon.name: "document-edit"
                visible: root.controller.trip && root.controller.trip.intermediateStops.length > 0 // TODO also check for subsequent layovers
                onClicked: alightSheet.open();
            }
        }

        // TODO the below is per reservation, not per batch, so add a selector for that!

        FormCard.FormHeader {
            title: i18nc("Train seat", "Seat")
        }

        FormCard.FormCard {
            FormCard.FormTextFieldDelegate {
                id: coachEdit
                label: i18nc("carriage on a train", "Coach")
                text: reservation.reservedTicket.ticketedSeat.seatSection
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormTextFieldDelegate {
                id: seatEdit
                label: i18nc("Train seat", "Seat")
                text: reservation.reservedTicket.ticketedSeat.seatNumber
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormTextFieldDelegate {
                id: classEdit
                label: i18nc("seating class on a train", "Class")
                text: reservation.reservedTicket.ticketedSeat.seatingType
            }
        }

        ProgramMembershipEditorCard {
            id: programMembershipEdit
            item: root.reservation
        }

        BookingEditorCard {
            id: bookingEdit
            item: root.reservation
        }

        TicketTokenEditorCard {
            id: ticketTokenEdit
            item: root.reservation.reservedTicket
        }
    }
}
