/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kpublictransport 1.0 as KPublicTransport
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.DetailsPage {
    id: root
    title: i18n("Train Ticket")
    editor: Component {
        App.TrainEditor {
            controller: root.controller
        }
    }

    actions.main: Kirigami.Action {
        icon.name: "view-barcode-qr"
        text: i18n("Barcode Scan Mode")
        onTriggered: scanModeController.toggle()
        visible: ticketToken.hasBarcode
        checkable: true
        checked: scanModeController.enabled
    }

    Component {
        id: alternativePage
        App.AlternativeJourneyPage {
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

    BarcodeScanModeController {
        id: scanModeController
        page: root
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
            id: ticketToken
            resIds: ReservationManager.reservationsForBatch(root.batchId)
            onCurrentReservationIdChanged: {
                if (!currentReservationId)
                    return;
                root.currentReservationId = currentReservationId;
            }
            onScanModeToggled: scanModeController.toggle()
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true

            // departure data
            Kirigami.Separator {
                Kirigami.FormData.isSection: true
                Kirigami.FormData.label: i18n("Departure")
            }
            RowLayout {
                Kirigami.FormData.label: i18n("Departure time:")
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
                Kirigami.FormData.label: i18nc("train station", "Station:")
                text: reservationFor.departureStation.name
            }
            App.PlaceDelegate {
                place: reservationFor.departureStation
                controller: root.controller
                isRangeBegin: true
            }
            RowLayout {
                Kirigami.FormData.label: i18n("Platform:")
                visible: departurePlatformLabel.text != ""
                QQC2.Label {
                    id: departurePlatformLabel
                    text: departure.hasExpectedPlatform ? departure.expectedPlatform : reservationFor.departurePlatform
                    color: departure.platformChanged ? Kirigami.Theme.negativeTextColor :
                        departure.hasExpectedPlatform ? Kirigami.Theme.positiveTextColor :
                        Kirigami.Theme.textColor;
                }
                QQC2.Label {
                    text: i18nc("previous platform", "(was: %1)", reservationFor.departurePlatform)
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
                onLinkActivated: Qt.openUrlExternally(link)
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
                Kirigami.FormData.label: i18nc("train station", "Station:")
                text: reservationFor.arrivalStation.name
            }
            App.PlaceDelegate {
                place: reservationFor.arrivalStation
                controller: root.controller
                isRangeEnd: true
            }
            RowLayout {
                Kirigami.FormData.label: i18n("Platform:")
                visible: arrivalPlatformLabel.text != ""
                QQC2.Label {
                    id: arrivalPlatformLabel
                    text: arrival.hasExpectedPlatform ? arrival.expectedPlatform : reservationFor.arrivalPlatform
                    color: arrival.platformChanged ? Kirigami.Theme.negativeTextColor :
                        arrival.hasExpectedPlatform ? Kirigami.Theme.positiveTextColor :
                        Kirigami.Theme.textColor;
                }
                QQC2.Label {
                    text: i18nc("previous platform", "(was: %1)", reservationFor.arrivalPlatform)
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
                onLinkActivated: Qt.openUrlExternally(link)
            }

            // seat reservation
            Kirigami.Separator {
                Kirigami.FormData.label: i18n("Seat")
                Kirigami.FormData.isSection: true
                visible: coachLabel.visible || seatLabel.visible || classLabel.visible
            }
            QQC2.Label {
                id: coachLabel
                Kirigami.FormData.label: i18nc("coach of a train", "Coach:")
                text: currentReservation.reservedTicket.ticketedSeat.seatSection
                visible: text
            }
            QQC2.Label {
                id: seatLabel
                Kirigami.FormData.label: i18n("Seat:")
                text: currentReservation.reservedTicket.ticketedSeat.seatNumber
                visible: text
            }
            QQC2.Label {
                id: classLabel
                Kirigami.FormData.label: i18n("Class:")
                text: currentReservation.reservedTicket.ticketedSeat.seatingType
                visible: text
            }

            // program membership
            Kirigami.Separator {
                Kirigami.FormData.label: i18nc("bonus, discount or frequent traveler program", "Program")
                Kirigami.FormData.isSection: true
                visible: programNameLabel.visible || membershipNumberLabel.visible
            }
            QQC2.Label {
                id: programNameLabel
                Kirigami.FormData.label: i18n("Name:")
                text: root.currentReservation.programMembershipUsed.programName
                visible: text
            }
            QQC2.Label {
                id: membershipNumberLabel
                Kirigami.FormData.label: i18n("Number:")
                text: root.currentReservation.programMembershipUsed.membershipNumber
                visible: text
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
