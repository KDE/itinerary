/*
    SPDX-FileCopyrightText: 2019-2023 Volker Krause <vkrause@kde.org>
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
    title: i18n("Edit Bus Reservation")

    property var departureBusStop: reservationFor.departureBusStop
    property var departureTime: Util.dateTimeStripTimezone(reservationFor, "departureTime")
    property var arrivalBusStop: reservationFor.arrivalBusStop
    property var arrivalTime: Util.dateTimeStripTimezone(reservationFor, "arrivalTime")

    function apply(reservation) {
        let trip = reservation.reservationFor;
        trip.departureBusStop = root.departureBusStop;
        trip = Util.setDateTimePreserveTimezone(trip, "departureTime", root.departureTime);
        trip.departurePlatform = departurePlatform.text;
        trip.arrivalBusStop = root.arrivalBusStop;
        trip = Util.setDateTimePreserveTimezone(trip, "arrivalTime", root.arrivalTime);
        trip.arrivalPlatform = arrivalPlatform.text;

        let ticket = reservation.reservedTicket ?? Factory.makeTicket();
        ticketTokenEdit.apply(ticket);
        let seat = ticket.ticketedSeat;
        seat.seatNumber = seatNumber.text;
        ticket.ticketedSeat = seat;

        var newRes = reservation;
        newRes.reservationFor = trip;
        newRes.reservedTicket = ticket;
        bookingEdit.apply(newRes);
        return newRes;
    }

    IntermediateStopSelector {
        id: boardSheet
        title: i18nc("bus departure", "Change Departure Stop")
        model: [root.controller.trip.departure].concat(root.controller.trip.intermediateStops).concat([root.controller.trip.arrival])
        currentIndex: root.controller.tripDepartureIndex
        disableAfterIndex: alightSheet.currentIndex
        forBoarding: true
        action: Kirigami.Action {
            text: i18nc("bus departure", "Change departure stop")
            onTriggered: {
                departureBusStop = PublicTransport.busStationFromLocation(root.controller.trip.stopover(boardSheet.currentIndex).stopPoint)
                departureTime = Util.dateTimeStripTimezone(root.controller.trip.stopover(boardSheet.currentIndex), "scheduledDepartureTime");
                boardSheet.close();
            }
        }
    }
    IntermediateStopSelector {
        id: alightSheet
        title: i18nc("bus arrival", "Change Arrival Stop")
        model: [root.controller.trip.departure].concat(root.controller.trip.intermediateStops).concat([root.controller.trip.arrival])
        currentIndex: root.controller.tripArrivalIndex
        disableBeforeIndex: boardSheet.currentIndex
        forBoarding: false
        action: Kirigami.Action {
            text: i18nc("bus arrival", "Change arrival stop")
            onTriggered: {
                arrivalBusStop = PublicTransport.busStationFromLocation(root.controller.trip.stopover(alightSheet.currentIndex).stopPoint);
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
            emojiIcon: "🚌"
            text: if (reservationFor.busNumber || reservationFor.busName) {
                return reservationFor.busName + " " + reservationFor.busNumber;
            } else {
                return i18nc("@title", "Bus")
            }
        }

        FormCard.FormHeader {
            title: i18nc("bus departure", "Departure")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                text: i18nc("bus stop", "Stop")
                description: root.departureBusStop.name
            }
            FormCard.FormTextFieldDelegate {
                id: departurePlatform
                label: i18nc("bus stop platform", "Platform")
                text: reservationFor.departurePlatform
            }
            FormCard.FormButtonDelegate {
                text: i18nc("bus departure", "Change Departure Stop")
                icon.name: "document-edit"
                visible: root.controller.trip && root.controller.trip.intermediateStops.length > 0 // TODO also check for preceding layovers
                onClicked: boardSheet.open();
            }
        }

        FormCard.FormHeader {
            title: i18nc("bus arrival", "Arrival")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                text: i18nc("bus stop", "Stop")
                description: root.arrivalBusStop.name
            }
            FormCard.FormTextFieldDelegate {
                id: arrivalPlatform
                label: i18nc("bus stop platform", "Platform")
                text: reservationFor.arrivalPlatform
            }
            FormCard.FormButtonDelegate {
                text: i18nc("bus arrival", "Change Arrival Stop")
                icon.name: "document-edit"
                visible: root.controller.trip && root.controller.trip.intermediateStops.length > 0 // TODO also check for subsequent layovers
                onClicked: alightSheet.open();
            }
        }

        // TODO the below is per reservation, not per batch, so add a selector for that!
        FormCard.FormHeader {
            title: i18nc("Bus seat", "Seat")
        }

        FormCard.FormCard {
            FormCard.FormTextFieldDelegate {
                id: seatNumber
                label: i18nc("Bus seat", "Seat")
                text: reservation.reservedTicket ? reservation.reservedTicket.ticketedSeat.seatNumber : ""
            }
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
