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

    // fully manually edited or backed by KPublicTransport data?
    readonly property bool isManual: !root.controller || root.controller.trip.mode === JourneySection.Invalid

    property var departureStation: reservationFor.departureStation
    property var departureTime: Util.dateTimeStripTimezone(reservationFor, "departureTime")
    property var arrivalStation: reservationFor.arrivalStation
    property var arrivalTime: Util.dateTimeStripTimezone(reservationFor, "arrivalTime")

    function apply(reservation) {
        let trip = reservation.reservationFor;

        if (root.isManual) {
            if (departureTimeEdit.isModified) {
                trip.departureDay = undefined;
                trip = Util.setDateTimePreserveTimezone(trip, "departureTime", departureTimeEdit.value);
            }
            if (arrivalTimeEdit.isModified) {
                trip = Util.setDateTimePreserveTimezone(trip, "arrivalTime", arrivalTimeEdit.value);
            }

            const departure = trip.departureStation;
            departure.name = departureStationName.text;
            departureAddress.save(departure);
            trip.departureStation = departure;
            trip.departurePlatform = departurePlatformNew.text

            const arrival = trip.arrivalStation;
            arrival.name = arrivalStationName.text;
            arrivalAddress.save(arrival);
            trip.arrivalStation = arrival;
            trip.arrivalPlatform = arrivalPlatformNew.text

            trip.trainName = '';
            trip.trainNumber = trainNumber.text;
        } else {
            trip.departureStation = root.departureStation;
            trip = Util.setDateTimePreserveTimezone(trip, "departureTime", root.departureTime);
            trip.departurePlatform = departurePlatform.text;

            trip.arrivalStation = root.arrivalStation;
            trip = Util.setDateTimePreserveTimezone(trip, "arrivalTime", root.arrivalTime);
            trip.arrivalPlatform = arrivalPlatform.text;
        }

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

    title: root.isNew ? i18nc("@title", "New Train Reservation") : i18nc("@title", "Edit Train Reservation")
    isValidInput: !isManual || (departureStationName.text !== "" && arrivalStationName.text !== "" && departureTimeEdit.hasValue
        && (!arrivalTimeEdit.hasValue || departureTimeEdit.value < arrivalTimeEdit.value))

    tripGroupSelector: tripGroupSelector


    IntermediateStopSelector {
        id: boardSheet
        title: i18nc("train departure", "Change Departure Station")
        model: [root.controller.trip.departure].concat(root.controller.trip.intermediateStops).concat([root.controller.trip.arrival])
        currentIndex: root.controller.tripDepartureIndex
        disableAfterIndex: alightSheet.currentIndex
        forBoarding: true
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
        currentIndex: root.controller.tripArrivalIndex
        disableBeforeIndex: boardSheet.currentIndex
        forBoarding: false
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
            title: i18n("Trip")
            visible: root.isNew
        }

        TripGroupSelectorCard {
            id: tripGroupSelector

            visible: root.isNew
            suggestedName: arrivalStationName.text
            tripGroupCandidates: TripGroupModel.intersectingXorAdjacentTripGroups(departureTimeEdit.value, arrivalTimeEdit.hasValue ? arrivalTimeEdit.value : departureTimeEdit.value)
        }

        FormCard.FormHeader {
            visible: root.isManual
            title: i18nc("@title:group", "Train")
        }

        FormCard.FormCard {
            visible: root.isManual

            FormCard.FormTextFieldDelegate {
                id: trainNumber

                label: i18nc("@label:textfield", "Train number")
                text: (reservationFor.trainName.length > 0 ? reservationFor.trainName + ' ' : '') + reservationFor.trainNumber
            }
        }

        FormCard.FormHeader {
            title: i18nc("train departure", "Departure")
        }

        FormCard.FormCard {
            visible: root.isManual

            FormDateTimeEditDelegate {
                id: departureTimeEdit

                text: i18nc("Train departure", "Departure Time")
                obj: reservation.reservationFor
                propertyName: "departureTime"
                status: Kirigami.MessageType.Error
                statusMessage: departureTimeEdit.hasValue ? '' : i18n("Departure time has to be set.")
                initialDay: Util.dateTimeStripTimezone(root.reservation.reservationFor, "departureDay")
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextFieldDelegate {
                id: departureStationName

                label: i18nc("Train station platform", "Station Name")
                text: reservation.reservationFor.departureStation.name
                status: Kirigami.MessageType.Error
                statusMessage: text === "" ? i18n("Departure station must not be empty.") : ""
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextFieldDelegate {
                id: departurePlatformNew

                label: i18nc("Train station platform", "Platform")
                text: reservation.reservationFor.departurePlatform
            }

            FormCard.FormDelegateSeparator {}

            Kirigami.InlineMessage {
                type: Kirigami.InlineMessage.Information

                width: parent.width

                text: i18n("Specifying the departure and arrival location helps Itinerary to find updated information for the train this reservation is valid for.")

                visible: !departureAddress.place.geo.isValid
            }

            FormPlaceEditorDelegate {
                id: departureAddress

                place: {
                    if (root.batchId || !root.reservation.reservationFor.departureStation.address.isEmpty || root.reservation.reservationFor.departureStation.geo.isValid)
                        return reservation.reservationFor.departureStation;
                    return cityAtTime(root.reservation.reservationFor.departureTime);
                }
            }
        }

        FormCard.FormCard {
            visible: !root.isManual

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
            visible: root.isManual

            FormDateTimeEditDelegate {
                id: arrivalTimeEdit

                text: i18nc("Train arrival", "Arrival Time")
                obj: reservation.reservationFor
                propertyName: "arrivalTime"
                initialValue: {
                    let d = new Date(departureTimeEdit.value);
                    d.setHours(d.getHours() + 4);
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
                id: arrivalStationName

                label: i18nc("Train station", "Station Name")
                text: root.arrivalStation.name
                status: Kirigami.MessageType.Error
                statusMessage: text === "" ? i18n("Arrival station must not be empty.") : ""
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextFieldDelegate {
                id: arrivalPlatformNew

                label: i18nc("Train station platform", "Platform")
                text: reservation.reservationFor.arrivalPlatform
            }

            FormCard.FormDelegateSeparator {}

            Kirigami.InlineMessage {
                type: Kirigami.InlineMessage.Information

                width: parent.width

                text: i18n("Specifying the departure and arrival location helps Itinerary to find updated information for the train this reservation is valid for.")

                visible: !arrivalAddress.place.geo.isValid
            }

            FormPlaceEditorDelegate {
                id: arrivalAddress

                place: {
                    if (root.batchId || !root.reservation.reservationFor.arrivalStation.address.isEmpty || root.reservation.reservationFor.arrivalStation.geo.isValid)
                        return reservation.reservationFor.arrivalStation;
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
                text: reservation.reservedTicket?.ticketedSeat?.seatSection
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormTextFieldDelegate {
                id: seatEdit
                label: i18nc("Train seat", "Seat")
                text: reservation.reservedTicket?.ticketedSeat?.seatNumber
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormTextFieldDelegate {
                id: classEdit
                label: i18nc("seating class on a train", "Class")
                text: reservation.reservedTicket?.ticketedSeat?.seatingType
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
