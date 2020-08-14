/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.DetailsPage {
    id: root
    title: i18n("Bus Ticket")

    Component {
        id: alternativePage
        App.JourneyQueryPage {
            controller: root.controller
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

    Component.onCompleted: {
        actions.contextualActions.push(alternativeAction.createObject(root));
    }

    ColumnLayout {
        width: parent.width

        QQC2.Label {
            Layout.fillWidth: true
            text: reservationFor.busName + " " + reservationFor.busNumber
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
            QQC2.Label {
                Kirigami.FormData.label: i18n("Time:")
                text: Localizer.formatDateTime(reservationFor, "departureTime")
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Station:")
                text: reservationFor.departureBusStop.name
            }
            App.PlaceDelegate {
                place: reservationFor.departureBusStop
                controller: root.controller
                isRangeBegin: true
            }

            // arrival data
            Kirigami.Separator {
                Kirigami.FormData.label: i18n("Arrival")
                Kirigami.FormData.isSection: true
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Time:")
                text: Localizer.formatDateTime(reservationFor, "arrivalTime")
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Station:")
                text: reservationFor.arrivalBusStop.name
            }
            App.PlaceDelegate {
                place: reservationFor.arrivalBusStop
                controller: root.controller
                isRangeEnd: true
            }

            // seat reservation
            Kirigami.Separator {
                Kirigami.FormData.label: i18n("Seat")
                Kirigami.FormData.isSection: true
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("Seat:")
                text: reservation.reservedTicket.ticketedSeat.seatNumber
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
