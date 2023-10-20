/*
    SPDX-FileCopyrightText: 2019-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kitinerary 1.0
import org.kde.kpublictransport 1.0
import org.kde.itinerary 1.0
import "." as App

App.EditorPage {
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
        let seat = ticket.ticketedSeat;
        seat.seatNumber = seatNumber.text;
        ticket.ticketedSeat = seat;

        var newRes = reservation;
        newRes.reservationFor = trip;
        newRes.reservedTicket = ticket;
        bookingEdit.apply(newRes);
        return newRes;
    }

    App.IntermediateStopSelector {
        id: boardSheet
        title: i18n("Board Later")
        model: root.controller.journey.intermediateStops
        action: Kirigami.Action {
            text: i18n("Change departure stop")
            onTriggered: {
                departureBusStop = PublicTransport.busStationFromLocation(root.controller.journey.intermediateStops[boardSheet.currentIndex].stopPoint)
                departureTime = Util.dateTimeStripTimezone(root.controller.journey.intermediateStops[boardSheet.currentIndex], "scheduledDepartureTime");
                boardSheet.close();
            }
        }
    }
    App.IntermediateStopSelector {
        id: alightSheet
        title: i18n("Alight Earlier")
        model: root.controller.journey.intermediateStops
        action: Kirigami.Action {
            text: i18n("Change arrival stop")
            onTriggered: {
                arrivalBusStop = PublicTransport.busStationFromLocation(root.controller.journey.intermediateStops[alightSheet.currentIndex].stopPoint);
                arrivalTime = Util.dateTimeStripTimezone(root.controller.journey.intermediateStops[alightSheet.currentIndex], "scheduledArrivalTime");
                if (!arrivalTime) {
                    arrivalTime = Util.dateTimeStripTimezone(root.controller.journey.intermediateStops[alightSheet.currentIndex], "scheduledDepartureTime");
                }
                alightSheet.close();
            }
        }
    }

    ColumnLayout {
        spacing: 0

        App.CardPageTitle {
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
                text: i18n("Board later")
                icon.name: "arrow-right"
                visible: root.controller.journey && root.controller.journey.intermediateStops.length > 0 // TODO also check for preceding layovers
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
                text: i18n("Alight earlier")
                icon.name: "arrow-left"
                visible: root.controller.journey && root.controller.journey.intermediateStops.length > 0 // TODO also check for subsequent layovers
                onClicked: alightSheet.open();
            }
        }

        // TODO the below is per reservation, not per batch, so add a selector for that!
        FormCard.FormHeader {
            title: i18n("Seat")
        }

        FormCard.FormCard {
            FormCard.FormTextFieldDelegate {
                id: seatNumber
                label: i18n("Seat")
                text: reservation.reservedTicket ? reservation.reservedTicket.ticketedSeat.seatNumber : ""
            }
        }

        App.BookingEditorCard {
            id: bookingEdit
            item: root.reservation
        }
    }
}
