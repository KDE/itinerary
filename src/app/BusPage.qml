/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kitinerary
import org.kde.itinerary
import org.kde.kirigamiaddons.formcard as FormCard

DetailsPage {
    id: root

    title: i18n("Bus Ticket")

    editor: BusEditor {
        controller: root.controller
    }

    data: BarcodeScanModeButton {
        page: root
        visible: ticketToken.hasBarcode
    }

    ColumnLayout {
        spacing: 0

        CardPageTitle {
            emojiIcon: "🚌"
            text: reservationFor.busName + " " + reservationFor.busNumber
        }

        FormCard.FormCard {
            visible: ticketToken.ticketTokenCount > 0
            // ticket barcode
            TicketTokenDelegate {
                id: ticketToken
                resIds: ReservationManager.reservationsForBatch(root.batchId)
                onCurrentReservationIdChanged: {
                    if (!currentReservationId)
                        return;
                    root.currentReservationId = currentReservationId;
                }
                onScanModeToggled: scanModeController.toggle()
                visible: ticketToken.ticketTokenCount > 0
            }
        }

        // departure data
        FormCard.FormHeader {
            title: i18nc("Bus departure", "Departure")
        }

        FormCard.FormCard {
            FormCard.AbstractFormDelegate {
                id: departureTimeDelegate
                visible: departureTimeLabel.text.length > 0
                background: null
                contentItem: ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing
                    QQC2.Label {
                        Layout.fillWidth: true
                        text: i18n("Departure time")
                        elide: Text.ElideRight
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        QQC2.Label {
                            id: departureTimeLabel
                            text: Localizer.formatDateTime(reservationFor, "departureTime")
                            color: Kirigami.Theme.disabledTextColor
                            elide: Text.ElideRight
                        }
                        QQC2.Label {
                            text: (departure.departureDelay >= 0 ? "+" : "") + departure.departureDelay
                            color: (departure.departureDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                            visible: departure.hasExpectedDepartureTime
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
                text: i18nc("bus station", "Station")
                description: reservationFor.departureBusStop.name
            }

            FormCard.FormDelegateSeparator { visible: departurePlatformDelegate.visible }

            FormCard.FormTextDelegate {
                id: departurePlatformDelegate
                text: i18nc("bus station platform", "Platform")
                description: reservationFor.departurePlatform
                visible: description !== ""
            }

            FormCard.FormDelegateSeparator { visible: departureDelegate.visible }

            FormPlaceDelegate {
                id: departureDelegate
                place: reservationFor.departureBusStop
                controller: root.controller
                isRangeBegin: true
            }
        }

        // arrival data
        FormCard.FormHeader {
            title: i18nc("Bus arrival", "Arrival")
        }

        FormCard.FormCard {
            FormCard.AbstractFormDelegate {
                background: null
                visible: reservationFor.arrivalTime > 0
                contentItem: ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing
                    QQC2.Label {
                        Layout.fillWidth: true
                        text: i18n("Arrival time")
                        elide: Text.ElideRight
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        QQC2.Label {
                            id: arrivalTimeLabel
                            text: Localizer.formatDateTime(reservationFor, "arrivalTime")
                            color: Kirigami.Theme.disabledTextColor
                            elide: Text.ElideRight
                        }
                        QQC2.Label {
                            font: Kirigami.Theme.smallFont
                            text: (arrival.arrivalDelay >= 0 ? "+" : "") + arrival.arrivalDelay
                            color: (arrival.arrivalDelay > 1) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.positiveTextColor
                            visible: arrival.hasExpectedArrivalTime
                        }
                    }
                }
            }

            FormCard.FormDelegateSeparator { visible: arrivalTimeLabel.text.length > 0 }

            FormCard.FormTextDelegate {
                text: i18nc("bus station", "Station")
                description: reservationFor.arrivalBusStop.name
            }

            FormCard.FormDelegateSeparator { visible: arrivalPlatformDelegate.visible }

            FormCard.FormTextDelegate {
                id: arrivalPlatformDelegate
                text: i18nc("bus station platform", "Platform")
                description: reservationFor.arrivalPlatform
                visible: description !== ""
            }

            FormCard.FormDelegateSeparator { visible: arrivalDelegate.visible }

            FormPlaceDelegate {
                id: arrivalDelegate
                place: reservationFor.arrivalBusStop
                controller: root.controller
                isRangeEnd: true
            }
        }

        // seat reservation
        FormCard.FormHeader {
            title: i18n("Seat")
        }

        FormCard.FormCard {
            visible: seatLabel.visible
            FormCard.FormTextDelegate {
                id: seatLabel
                text: root.reservation.reservedTicket ? root.reservation.reservedTicket.ticketedSeat.seatNumber : ""
                visible: text !== ""
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
                }
            ]

            Component {
                id: alternativePage
                AlternativeJourneyPage {
                    controller: root.controller
                    publicTransportManager: LiveDataManager.publicTransportManager
                }
            }
        }
    }
}
