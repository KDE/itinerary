// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kpublictransport as KPublicTransport
import org.kde.kitinerary
import org.kde.itinerary

DetailsPage {
    id: root

    title: i18n("Train Ticket")
    editor: TrainEditor {
        controller: root.controller
    }

    data: BarcodeScanModeButton {
        page: root
        visible: ticketToken.hasBarcode
    }

    Component {
        id: alternativePage
        AlternativeJourneyPage {
            controller: root.controller
            publicTransportManager: LiveDataManager.publicTransportManager
        }
    }

    Component {
        id: vehicleLayoutPage
        VehicleLayoutPage {
            publicTransportManager: root.controller.liveDataManager.publicTransportManager
            selectedVehicleSection: root.reservation.reservedTicket.ticketedSeat.seatSection
            selectedClasses: root.reservation.reservedTicket.ticketedSeat.seatingType
            seat: root.reservation.reservedTicket.ticketedSeat.seatNumber

            property bool arrival

            onLayoutUpdated: root.controller.setVehicleLayout(vehicleLayout, arrival);
        }
    }

    ColumnLayout {
        spacing: 0

        CardPageTitle {
            emojiIcon: "🚅"
            text: {
                if (reservationFor.trainName || reservationFor.trainNumber) {
                    return reservationFor.trainName + " " + reservationFor.trainNumber
                }
                return i18n("%1 to %2", reservationFor.departureStation.name, reservationFor.arrivalStation.name);
            }
        }

        FormCard.FormCard {
            visible: ticketToken.ticketTokenCount > 0
            // ticket barcode
            TicketTokenDelegate {
                id: ticketToken
                Layout.fillWidth: true
                resIds: ReservationManager.reservationsForBatch(root.batchId)
                onCurrentReservationIdChanged: {
                    if (!currentReservationId)
                        return;
                    root.currentReservationId = currentReservationId;
                }
                onScanModeToggled: scanModeController.toggle()
            }
        }

        // departure data
        FormCard.FormHeader {
            title: i18nc("Train departure", "Departure")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                id: departureTimeDelegate
                text: i18n("Departure time")
                visible: departureTimeLabel.text.length > 0
                contentItem: ColumnLayout {
                    spacing: Kirigami.Units.smallSpacing
                    QQC2.Label {
                        text: departureTimeDelegate.text
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        Accessible.ignored: true
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        QQC2.Label {
                            id: departureTimeLabel
                            text: Localizer.formatDateTime(reservationFor, "departureTime")
                            color: Kirigami.Theme.disabledTextColor
                            font: Kirigami.Theme.smallFont
                            elide: Text.ElideRight
                        }
                        QQC2.Label {
                            text: (departure.departureDelay >= 0 ? "+" : "") + departure.departureDelay
                            color: (departure.departureDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                            visible: departure.hasExpectedDepartureTime
                            Accessible.ignored: !visible
                        }
                    }
                }
            }
            FormCard.FormTextDelegate {
                text: i18n("Departure date")
                visible: !departureTimeDelegate.visible && text.length > 0
                description: Localizer.formatDate(reservationFor, "departureDay")
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                text: i18nc("train station", "Station")
                description: reservationFor.departureStation.name
                visible: description
            }

            FormCard.FormDelegateSeparator { visible: reservationFor.departureStation.name }

            FormPlatformDelegate {
                id: departurePlatformDelegate
                stopover: departure
                sections: root.controller.departurePlatformSections
                scheduledPlatform: reservationFor.departurePlatform
            }

            FormCard.FormDelegateSeparator { visible: departurePlatformDelegate.visible }

            FormPlaceDelegate {
                id: departureDelegate
                place: reservationFor.departureStation
                controller: root.controller
                isRangeBegin: true
            }

            FormCard.FormDelegateSeparator {
                visible: departureDelegate.visible
            }

            FormCard.FormTextDelegate {
                text: i18n("Additional notes")
                description: departure.notes.join("<br/>")
                descriptionItem.textFormat: Text.RichText
                descriptionItem.wrapMode: Text.Wrap
                visible: arrival.notes.length > 0
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }

        // arrival data
        FormCard.FormHeader {
            title: i18nc("Train arrival", "Arrival")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                id: arrivalTimeDelegate
                text: i18n("Arrival time")
                visible: arrivalTimeLabel.text.length > 0
                contentItem: ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing
                    QQC2.Label {
                        text: arrivalTimeDelegate.text
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        Accessible.ignored: true
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        QQC2.Label {
                            id: arrivalTimeLabel
                            text: Localizer.formatDateTime(reservationFor, "arrivalTime")
                            color: Kirigami.Theme.disabledTextColor
                            font: Kirigami.Theme.smallFont
                            elide: Text.ElideRight
                        }
                        QQC2.Label {
                            font: Kirigami.Theme.smallFont
                            text: (arrival.arrivalDelay >= 0 ? "+" : "") + arrival.arrivalDelay
                            color: (arrival.arrivalDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                            visible: arrival.hasExpectedArrivalTime
                            Accessible.ignored: !visible
                        }
                    }
                }
            }

            FormCard.FormDelegateSeparator { visible: arrivalTimeLabel.text.length > 0 }

            FormCard.FormTextDelegate {
                text: i18nc("train station", "Station")
                description: reservationFor.arrivalStation.name
            }

            FormCard.FormDelegateSeparator { visible: reservationFor.arrivalStation.name }

            FormPlatformDelegate {
                id: arrivalPlatformDelegate
                stopover: arrival
                sections: root.controller.arrivalPlatformSections
                scheduledPlatform: reservationFor.arrivalPlatform
            }

            FormCard.FormDelegateSeparator { visible: arrivalPlatformDelegate.visible }

            FormPlaceDelegate {
                id: arrivalDelegate
                place: reservationFor.arrivalStation
                controller: root.controller
                isRangeEnd: true
            }

            FormCard.FormDelegateSeparator {
                visible: arrivalDelegate
            }

            FormCard.FormTextDelegate {
                text: i18n("Additional notes")
                description: arrival.notes.join("<br/>")
                descriptionItem.textFormat: Text.RichText
                descriptionItem.wrapMode: Text.Wrap
                visible: arrival.notes.length > 0
                font.italic: true
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }

        // seat reservation
        FormCard.FormHeader {
            visible: seatCard.visible
            title: i18n("Seat")
        }

        FormCard.FormCard {
            id: seatCard
            visible: coachLabel.visible || seatLabel.visible || classLabel.visible || departureLayoutButton.visible || arrivalLayoutButton.visible
            FormCard.FormTextDelegate {
                id: coachLabel
                text: i18nc("coach of a train", "Coach:")
                description: root.reservation.reservedTicket ? root.reservation.reservedTicket.ticketedSeat.seatSection : ''
                visible: description
            }
            FormCard.FormDelegateSeparator {
                visible: root.reservation.reservedTicket ? root.reservation.reservedTicket.ticketedSeat.seatSection : false
            }
            FormCard.FormTextDelegate {
                id: seatLabel
                text: i18n("Seat:")
                description: root.reservation.reservedTicket ? root.reservation.reservedTicket.ticketedSeat.seatNumber : ''
                visible: description
            }
            FormCard.FormDelegateSeparator {
                visible: root.reservation.reservedTicket ? root.reservation.reservedTicket.ticketedSeat.seatNumber : false
            }
            FormCard.FormTextDelegate {
                id: classLabel
                text: i18n("Class:")
                description: root.reservation.reservedTicket ? root.reservation.reservedTicket.ticketedSeat.seatingType : ''
                visible: description
            }
            FormCard.FormDelegateSeparator {
                visible: classLabel.visible
            }
            FormCard.FormButtonDelegate {
                id: departureLayoutButton
                text: i18n("Departure Vehicle Layout")
                icon.name: "view-list-symbolic"
                enabled: departure && (departure.route.line.mode == KPublicTransport.Line.LongDistanceTrain || departure.route.line.mode == KPublicTransport.Line.Train || departure.route.name !== "")
                onClicked: applicationWindow().pageStack.push(vehicleLayoutPage, {stopover: root.controller.departure, arrival: false})
                visible: enabled
            }
            FormCard.FormButtonDelegate {
                id: arrivalLayoutButton
                text: i18n("Arrival Vehicle Layout")
                icon.name: "view-list-symbolic"
                enabled: arrival && (arrival.route.line.mode == KPublicTransport.Line.LongDistanceTrain || arrival.route.line.mode == KPublicTransport.Line.Train || arrival.route.name !== "")
                onClicked: applicationWindow().pageStack.push(vehicleLayoutPage, {stopover: root.controller.arrival, arrival: true});
                visible: enabled

            }
        }

        ProgramMembershipCard {
            programMembership: root.reservation.programMembershipUsed
        }

        BookingCard {
            reservation: root.reservation
        }

        ReservationDocumentsCard {
            controller: root.controller
        }

        ActionsCard {
            batchId: root.batchId
            editor: root.editor
            reservation: root.reservation
            additionalActions: [
                QQC2.Action {
                    text: i18n("Alternatives")
                    icon.name: "clock"
                    onTriggered: applicationWindow().pageStack.push(alternativePage)
                },
                Kirigami.Action {
                    text: i18n("Journey Details")
                    icon.name: "view-calendar-day"
                    onTriggered: applicationWindow().pageStack.push(journeySectionPage, {"journeySection": root.controller.journey});
                    Component.onCompleted: {
                        visible = Qt.binding(function() { return root.controller.journey && root.controller.journey.intermediateStops.length > 0});
                    }
                },
                Kirigami.Action {
                    text: "Test Notification"
                    icon.name: "notifications"
                    visible: Settings.developmentMode
                    onTriggered: LiveDataManager.showNotification(root.batchId)
                }
            ]
        }
    }
}
