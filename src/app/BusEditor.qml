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

    // fully manually edited or backed by KPublicTransport data?
    readonly property bool isManual: !root.controller || root.controller.trip.mode === JourneySection.Invalid

    property var departureBusStop: reservationFor.departureBusStop
    property var departureTime: Util.dateTimeStripTimezone(reservationFor, "departureTime")
    property var arrivalBusStop: reservationFor.arrivalBusStop
    property var arrivalTime: Util.dateTimeStripTimezone(reservationFor, "arrivalTime")

    function apply(reservation) {
        let trip = reservation.reservationFor;

        if (root.isManual) {
            if (departureTimeEdit.isModified) {
                trip = Util.setDateTimePreserveTimezone(trip, "departureTime", departureTimeEdit.value);
            }
            if (arrivalTimeEdit.isModified) {
                trip = Util.setDateTimePreserveTimezone(trip, "arrivalTime", arrivalTimeEdit.value);
            }

            const departure = trip.departureBusStop;
            departure.name = departureBusStopName.text;
            departureAddress.save(departure);
            trip.departureBusStop = departure;
            trip.departurePlatform = departurePlatformNew.text

            const arrival = trip.arrivalBusStop;
            arrival.name = arrivalBusStopName.text;
            arrivalAddress.save(arrival);
            trip.arrivalBusStop = arrival;
            trip.arrivalPlatform = arrivalPlatformNew.text

            trip.busName = '';
            trip.busNumber = busNumber.text;
        } else {
            trip.departureBusStop = root.departureBusStop;
            trip = Util.setDateTimePreserveTimezone(trip, "departureTime", root.departureTime);
            trip.departurePlatform = departurePlatform.text;
            trip.arrivalBusStop = root.arrivalBusStop;
            trip = Util.setDateTimePreserveTimezone(trip, "arrivalTime", root.arrivalTime);
            trip.arrivalPlatform = arrivalPlatform.text;
        }

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

    title: root.isNew ? i18nc("@title", "New Bus Reservation") : i18nc("@title", "Edit Bus Reservation")
    isValidInput: !isManual || (departureBusStopName.text !== "" && arrivalBusStopName.text !== "" && departureTimeEdit.hasValue
        && (!arrivalTimeEdit.hasValue || departureTimeEdit.value < arrivalTimeEdit.value))
    tripGroupSelector: tripGroupSelector

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
            title: i18n("Trip")
            visible: root.isNew
        }

        TripGroupSelectorCard {
            id: tripGroupSelector
            visible: root.isNew
            suggestedName: arrivalBusStopName.text
            tripGroupCandidates: TripGroupModel.intersectingXorAdjacentTripGroups(departureTimeEdit.value, arrivalTimeEdit.hasValue ? arrivalTimeEdit.value : departureTimeEdit.value)
        }

        FormCard.FormHeader {
            visible: root.isManual
            title: i18nc("@title:group", "Bus")
        }

        FormCard.FormCard {
            visible: root.isManual

            FormCard.FormTextFieldDelegate {
                id: busNumber

                label: i18nc("@label:textfield", "Bus number")
                text: (reservationFor.busName.length > 0 ? reservationFor.busName + ' ' : '') + reservationFor.busNumber
            }
        }

        FormCard.FormHeader {
            title: i18nc("bus departure", "Departure")
        }

        FormCard.FormCard {
            visible: root.isManual

            FormDateTimeEditDelegate {
                id: departureTimeEdit

                text: i18nc("Bus departure", "Departure Time")
                obj: reservation.reservationFor
                propertyName: "departureTime"
                status: Kirigami.MessageType.Error
                statusMessage: departureTimeEdit.hasValue ? '' : i18n("Departure time has to be set.")
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextFieldDelegate {
                id: departureBusStopName

                label: i18nc("Bus stop", "Stop")
                text: reservation.reservationFor.departureBusStop.name
                status: Kirigami.MessageType.Error
                statusMessage: text === "" ? i18n("Departure bus stop must not be empty.") : ""
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextFieldDelegate {
                id: departurePlatformNew

                label: i18nc("Bus stop platform", "Platform")
                text: reservation.reservationFor.departurePlatform
            }

            FormCard.FormDelegateSeparator {}

            FormPlaceEditorDelegate {
                id: departureAddress

                place: {
                    if (root.batchId || !root.reservation.reservationFor.departureBusStop.address.isEmpty || root.reservation.reservationFor.departureBuStop.geo.isValid)
                        return reservation.reservationFor.departureBusStop;
                    return cityAtTime(root.reservation.reservationFor.departureTime);
                }
            }
        }

        FormCard.FormCard {
            visible: !root.isManual

            FormCard.FormTextDelegate {
                text: i18nc("bus stop", "Stop Name")
                description: root.departureBusStop.name
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextFieldDelegate {
                id: departurePlatform
                label: i18nc("bus stop platform", "Platform")
                text: reservationFor.departurePlatform
            }

            FormCard.FormDelegateSeparator { above: boardLater }

            FormCard.FormButtonDelegate {
                id: boardLater

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
            visible: root.isManual

            FormDateTimeEditDelegate {
                id: arrivalTimeEdit

                text: i18nc("Train arrival", "Arrival Time")
                obj: reservation.reservationFor
                propertyName: "arrivalTime"
                initialValue: {
                    let d = new Date(departureTimeEdit.value);
                    d.setHours(d.getHours() + 8);
                    return d;
                }
                status: Kirigami.MessageType.Error
                statusMessage: {
                    if (arrivalTimeEdit.hasValue && arrivalTimeEdit.value < departureTimeEdit.value)
                        return i18n("Arrival time has to be after the departure time.")
                    return '';
                }
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextFieldDelegate {
                id: arrivalBusStopName

                label: i18nc("Bus stop", "Stop Name")
                text: root.arrivalBusStop.name
                status: Kirigami.MessageType.Error
                statusMessage: text === "" ? i18n("Arrival stop must not be empty.") : ""
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextFieldDelegate {
                id: arrivalPlatformNew

                label: i18nc("Bus stop platform", "Platform")
                text: reservation.reservationFor.arrivalPlatform
            }

            FormCard.FormDelegateSeparator {}

            FormPlaceEditorDelegate {
                id: arrivalAddress

                place: {
                    if (root.batchId || !root.reservation.reservationFor.arrivalBusStop.address.isEmpty || root.reservation.reservationFor.arrivalBusStop.geo.isValid)
                        return reservation.reservationFor.arrivalBusStop;
                    let p = cityAtTime(root.reservation.reservationFor.departureTime);
                    let addr = p.address;
                    addr.addressLocality = undefined;
                    addr.addressRegion = undefined;
                    p.address = addr;
                    return p;
                }
            }
        }

        FormCard.FormCard {
            visible: !root.isManual

            FormCard.FormTextDelegate {
                text: i18nc("bus stop", "Stop")
                description: root.arrivalBusStop.name
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextFieldDelegate {
                id: arrivalPlatform
                label: i18nc("bus stop platform", "Platform")
                text: reservationFor.arrivalPlatform
            }

            FormCard.FormDelegateSeparator {}

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
                text: reservation.reservedTicket?.ticketedSeat?.seatNumber
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
