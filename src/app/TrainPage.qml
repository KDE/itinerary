/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.12 as Kirigami
import org.kde.kpublictransport 1.0 as KPublicTransport
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.DetailsPage {
    id: root
    title: i18n("Train Ticket")
    editor: Component {
        App.TrainEditor {
            batchId: root.batchId
        }
    }

    Component {
        id: alternativePage
        App.JourneyQueryPage {
            controller: root.controller
            publicTransportManager: LiveDataManager.publicTransportManager
        }
    }

    Component {
        id: vehicleLayoutPage
        App.VehicleLayoutPage {
            publicTransportManager: root.controller.liveDataManager.publicTransportManager
            selectedVehicleSection: root.reservation.reservedTicket.ticketedSeat.seatSection
            selectedClasses: root.reservation.reservedTicket.ticketedSeat.seatingType
            seat: root.reservation.reservedTicket.ticketedSeat.seatNumber
        }
    }

    Component {
        id: alternativeAction
        Kirigami.Action {
            text: i18n("Alternatives")
            iconName: "clock"
            onTriggered: {
                applicationWindow().pageStack.push(alternativePage);
            }
        }
    }
    Component {
        id: vehicleDepartureLayoutAction
        Kirigami.Action {
            text: i18n("Departure Vehicle Layout")
            iconName: "view-list-symbolic"
            enabled: departure && departure.route.line.mode == KPublicTransport.Line.LongDistanceTrain
            onTriggered: {
                applicationWindow().pageStack.push(vehicleLayoutPage, {"stopover": root.controller.departure});
            }
        }
    }
    Component {
        id: vehicleArrivalLayoutAction
        Kirigami.Action {
            text: i18n("Arrival Vehicle Layout")
            iconName: "view-list-symbolic"
            enabled: arrival && arrival.route.line.mode == KPublicTransport.Line.LongDistanceTrain
            onTriggered: {
                applicationWindow().pageStack.push(vehicleLayoutPage, {"stopover": root.controller.arrival});
            }
        }
    }
    Component {
        id: journeyDetailsAction
        Kirigami.Action {
            text: i18n("Journey Details")
            iconName: "view-calendar-day"
            onTriggered: applicationWindow().pageStack.push(journeySectionPage, {"journeySection": root.controller.journey});
            visible: root.controller.journey && root.controller.journey.intermediateStops.length > 0
        }
    }
    Component {
        id: notifyTestAction
        Kirigami.Action {
            text: "Test Notification"
            iconName: "notifications"
            visible: Settings.developmentMode
            onTriggered: LiveDataManager.showNotification(root.batchId)
        }
    }

    Component.onCompleted: {
        actions.contextualActions.push(alternativeAction.createObject(root));
        actions.contextualActions.push(journeyDetailsAction.createObject(root));
        actions.contextualActions.push(vehicleDepartureLayoutAction.createObject(root));
        actions.contextualActions.push(vehicleArrivalLayoutAction.createObject(root));
        actions.contextualActions.push(notifyTestAction.createObject(root));
    }

    ColumnLayout {
        width: parent.width

        QQC2.Label {
            Layout.fillWidth: true
            text: reservationFor.trainName + " " + reservationFor.trainNumber
            horizontalAlignment: Qt.AlignHCenter
            font.bold: true
        }

        // ticket barcode
        App.TicketTokenDelegate {
            resIds: ReservationManager.reservationsForBatch(root.batchId)
            onCurrentReservationIdChanged: {
                if (!currentReservationId)
                    return;
                root.currentReservationId = currentReservationId;
            }
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true

            // departure data
            Kirigami.Separator {
                Kirigami.FormData.isSection: true
                Kirigami.FormData.label: i18n("Departure")
            }
            RowLayout {
                Kirigami.FormData.label: i18n("Time:")
                QQC2.Label {
                    text: Localizer.formatDateTime(reservationFor, "departureTime")
                }
                QQC2.Label {
                    text: (departure.departureDelay >= 0 ? "+" : "") + departure.departureDelay
                    color: (departure.departureDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                    visible: departure.hasExpectedDepartureTime
                }
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Station:")
                text: reservationFor.departureStation.name
            }
            App.PlaceDelegate {
                place: reservationFor.departureStation
                controller: root.controller
                isRangeBegin: true
            }
            RowLayout {
                Kirigami.FormData.label: i18n("Platform:")
                QQC2.Label {
                    text: departure.hasExpectedPlatform ? departure.expectedPlatform : reservationFor.departurePlatform
                    color: departure.platformChanged ? Kirigami.Theme.negativeTextColor :
                        departure.hasExpectedPlatform ? Kirigami.Theme.positiveTextColor :
                        Kirigami.Theme.textColor;
                }
                QQC2.Label {
                    text: i18n("(was: %1)", reservationFor.departurePlatform)
                    visible: departure.platformChanged && reservationFor.departurePlatform != ""
                }
            }
            QQC2.Label {
                Kirigami.FormData.isSection: true
                text: departure.notes.join("<br/>")
                textFormat: Text.RichText
                wrapMode: Text.Wrap
                visible: departure.notes.length > 0
                font.italic: true
            }

            // arrival data
            Kirigami.Separator {
                Kirigami.FormData.isSection: true
                Kirigami.FormData.label: i18n("Arrival")
            }
            RowLayout {
                Kirigami.FormData.label: i18n("Arrival time:")
                QQC2.Label {
                    text: Localizer.formatDateTime(reservationFor, "arrivalTime")
                }
                QQC2.Label {
                    text: (arrival.arrivalDelay >= 0 ? "+" : "") + arrival.arrivalDelay
                    color: (arrival.arrivalDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                    visible: arrival.hasExpectedArrivalTime
                }
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Station:")
                text: reservationFor.arrivalStation.name
            }
            App.PlaceDelegate {
                place: reservationFor.arrivalStation
                controller: root.controller
                isRangeEnd: true
            }
            RowLayout {
                Kirigami.FormData.label: i18n("Platform:")
                QQC2.Label {
                    text: arrival.hasExpectedPlatform ? arrival.expectedPlatform : reservationFor.arrivalPlatform
                    color: arrival.platformChanged ? Kirigami.Theme.negativeTextColor :
                        arrival.hasExpectedPlatform ? Kirigami.Theme.positiveTextColor :
                        Kirigami.Theme.textColor;
                }
                QQC2.Label {
                    text: i18n("(was: %1)", reservationFor.arrivalPlatform)
                    visible: arrival.platformChanged && reservationFor.arrivalPlatform != ""
                }
            }
            QQC2.Label {
                Kirigami.FormData.isSection: true
                text: arrival.notes.join("<br/>")
                textFormat: Text.RichText
                wrapMode: Text.Wrap
                visible: arrival.notes.length > 0
                font.italic: true
            }

            // seat reservation
            Kirigami.Separator {
                Kirigami.FormData.label: i18n("Seat")
                Kirigami.FormData.isSection: true
                visible: reservation.reservedTicket.ticketedSeat.seatNumber != ""
                      || reservation.reservedTicket.ticketedSeat.seatSection != ""
                      || reservation.reservedTicket.ticketedSeat.seatingType != ""
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Coach:")
                text: reservation.reservedTicket.ticketedSeat.seatSection
                visible: reservation.reservedTicket.ticketedSeat.seatSection != ""
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Seat:")
                text: reservation.reservedTicket.ticketedSeat.seatNumber
                visible: reservation.reservedTicket.ticketedSeat.seatNumber != ""
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Class:")
                text: reservation.reservedTicket.ticketedSeat.seatingType
                visible: reservation.reservedTicket.ticketedSeat.seatingType != ""
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
}
