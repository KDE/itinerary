/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.i18n.localeData
import org.kde.kitinerary
import org.kde.kpublictransport as KPublicTransport
import org.kde.itinerary

EditorPage {
    id: root
    title: root.isNew ? i18n("New Boat Trip") : i18n("Edit Boat Trip")

    // fully manually edited or backed by KPublicTransport data?
    readonly property bool isManual: !root.controller || root.controller.trip.mode === KPublicTransport.JourneySection.Invalid

    isValidInput: !isManual || (departureTerminalName.text !== "" && arrivalTerminalName.text !== "" && departureTimeEdit.hasValue
        && (!arrivalTimeEdit.hasValue || departureTimeEdit.value < arrivalTimeEdit.value))
    tripGroupSelector: tripGroupSelector

    // departure/arrival when editing KPublicTransport-backed reservations
    property var departureBoatTerminal: root.reservation.reservationFor.departureBoatTerminal
    property var departureTime: Util.dateTimeStripTimezone(root.reservation.reservationFor, "departureTime")
    property var arrivalBoatTerminal: root.reservation.reservationFor.arrivalBoatTerminal
    property var arrivalTime: Util.dateTimeStripTimezone(root.reservation.reservationFor, "arrivalTime")

    function apply(reservation) {
        let trip = reservation.reservationFor;
        if (root.isManual) {
            if (departureTimeEdit.isModified)
                trip = Util.setDateTimePreserveTimezone(trip, "departureTime", departureTimeEdit.value);
            if (arrivalTimeEdit.isModified)
                trip = Util.setDateTimePreserveTimezone(trip, "arrivalTime", arrivalTimeEdit.value);

            let dep = trip.departureBoatTerminal;
            dep.name = departureTerminalName.text;
            departureAddress.save(dep);
            trip.departureBoatTerminal = dep;

            let arr = trip.arrivalBoatTerminal;
            arr.name = arrivalTerminalName.text;
            arrivalAddress.save(arr);
            trip.arrivalBoatTerminal = arr;
        } else {
            trip.departureBoatTerminal = root.departureBoatTerminal;
            trip = Util.setDateTimePreserveTimezone(trip, "departureTime", root.departureTime);
            trip.arrivalBoatTerminal = root.arrivalBoatTerminal;
            trip = Util.setDateTimePreserveTimezone(trip, "arrivalTime", root.arrivalTime);
        }

        let ticket = reservation.reservedTicket ?? Factory.makeTicket();
        ticketTokenEdit.apply(ticket);

        let newRes = reservation;
        newRes.reservationFor = trip;
        newRes.reservedTicket = ticket;
        bookingEdit.apply(newRes);
        return newRes;
    }

    IntermediateStopSelector {
        id: boardSheet
        title: i18nc("boat departure", "Change Departure Terminal")
        model: [root.controller.trip.departure].concat(root.controller.trip.intermediateStops).concat([root.controller.trip.arrival])
        currentIndex: root.controller.tripDepartureIndex
        disableAfterIndex: alightSheet.currentIndex
        forBoarding: true
        action: Kirigami.Action {
            text: i18nc("boat departure", "Change departure terminal")
            onTriggered: {
                departureBoatTerminal = PublicTransport.boatTerminalFromLocation(root.controller.trip.stopover(boardSheet.currentIndex).stopPoint)
                departureTime = Util.dateTimeStripTimezone(root.controller.trip.stopover(boardSheet.currentIndex), "scheduledDepartureTime");
                boardSheet.close();
            }
        }
    }
    IntermediateStopSelector {
        id: alightSheet
        title: i18nc("boat arrival", "Change Arrival Terminal")
        model: [root.controller.trip.departure].concat(root.controller.trip.intermediateStops).concat([root.controller.trip.arrival])
        currentIndex: root.controller.tripArrivalIndex
        disableBeforeIndex: boardSheet.currentIndex
        forBoarding: false
        action: Kirigami.Action {
            text: i18nc("boat arrival", "Change arrival terminal")
            onTriggered: {
                arrivalBoatTerminal = PublicTransport.boatTerminalFromLocation(root.controller.trip.stopover(alightSheet.currentIndex).stopPoint);
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
            emojiIcon: "🛳️"
            text: root.reservation?.reservationFor?.name.length > 0 ? root.reservation.reservationFor.name : i18nc("default transport name for a boat trip", "Ferry")
        }

        FormCard.FormHeader {
            title: i18n("Trip")
            visible: root.isNew
        }

        TripGroupSelectorCard {
            id: tripGroupSelector
            visible: root.isNew
            suggestedName: arrivalTerminalName.text
            tripGroupCandidates: TripGroupModel.intersectingXorAdjacentTripGroups(departureTimeEdit.value, arrivalTimeEdit.hasValue ? arrivalTimeEdit.value : departureTimeEdit.value)
        }

        FormCard.FormHeader {
            title: i18nc("Boat departure", "Departure")
        }

        FormCard.FormCard {
            visible: root.isManual
            FormDateTimeEditDelegate {
                id: departureTimeEdit
                text: i18nc("Boat departure", "Departure Time")
                obj: reservation.reservationFor
                propertyName: "departureTime"
                status: Kirigami.MessageType.Error
                statusMessage: departureTimeEdit.hasValue ? '' : i18n("Departure time has to be set.")
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormTextFieldDelegate {
                id: departureTerminalName
                label: i18nc("boat terminal", "Terminal Name")
                text: reservation.reservationFor.departureBoatTerminal.name
                status: Kirigami.MessageType.Error
                statusMessage: text === "" ? i18n("Departure terminal must not be empty.") : ""
            }
            FormCard.FormDelegateSeparator {}
            FormPlaceEditorDelegate {
                id: departureAddress
                place: {
                    if (root.batchId || !root.reservation.reservationFor.departureBoatTerminal.address.isEmpty || root.reservation.reservationFor.departureBoatTerminal.geo.isValid)
                        return reservation.reservationFor.departureBoatTerminal;
                    return cityAtTime(root.reservation.reservationFor.departureTime);
                }
            }
        }
        FormCard.FormCard {
            visible: !root.isManual
            FormCard.FormTextDelegate {
                text: i18nc("boat terminal", "Terminal")
                description: root.departureBoatTerminal.name
            }
            FormCard.FormDelegateSeparator { above: boardLater }
            FormCard.FormButtonDelegate {
                id: boardLater

                text: i18nc("boat departure", "Change Departure Terminal")
                icon.name: "document-edit"
                visible: root.controller.trip.intermediateStops.length > 0
                onClicked: boardSheet.open();
            }
        }

        FormCard.FormHeader {
            title: i18nc("Boat arrival", "Arrival")
        }

        FormCard.FormCard {
            visible: root.isManual
            FormDateTimeEditDelegate {
                id: arrivalTimeEdit
                text: i18nc("Boat arrival", "Arrival Time")
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
                id: arrivalTerminalName
                label: i18nc("boat terminal", "Terminal Name")
                text: reservation.reservationFor.arrivalBoatTerminal.name
                status: Kirigami.MessageType.Error
                statusMessage: text === "" ? i18n("Arrival terminal must not be empty.") : ""
            }
            FormCard.FormDelegateSeparator {}
            FormPlaceEditorDelegate {
                id: arrivalAddress
                place: {
                    if (root.batchId || !root.reservation.reservationFor.arrivalBoatTerminal.address.isEmpty || root.reservation.reservationFor.arrivalBoatTerminal.geo.isValid)
                        return reservation.reservationFor.arrivalBoatTerminal;
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
                text: i18nc("boat terminal", "Terminal")
                description: root.arrivalBoatTerminal.name
            }
            FormCard.FormDelegateSeparator { above: alignEarlier }
            FormCard.FormButtonDelegate {
                id: alignEarlier
                text: i18nc("boat arrival", "Change Arrival Terminal")
                icon.name: "document-edit"
                visible: root.controller.trip.intermediateStops.length > 0
                onClicked: alightSheet.open();
            }
        }

        BookingEditorCard {
            id: bookingEdit
            item: root.reservation
            defaultCurrency: Country.fromAlpha2(departureAddress.currentCountry).currencyCode
        }

        TicketTokenEditorCard {
            id: ticketTokenEdit
            item: root.reservation.reservedTicket
        }
    }
}
