// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport
import org.kde.kitinerary
import org.kde.itinerary

DetailsPage {
    id: root
    title: i18n("Flight")
    property var resIds: ReservationManager.reservationsForBatch(root.batchId)

    function airportDisplayString(airport): string {
        if (airport.name && airport.iataCode) {
            return airport.name + " (" + airport.iataCode + ")";
        } else {
            return airport.name || airport.iataCode || "";
        }
    }

    editor: FlightEditor {
        controller: root.controller
    }

    data: BarcodeScanModeButton {
        id: scanModeButton
        page: root
        visible: ticketToken.hasBarcode
    }

    ColumnLayout {
        spacing: 0

        KPublicTransport.TransportIcon {
            id: transportIcon
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            // A bit of extra spacing since the logos often have no padding.
            Layout.bottomMargin: root.departure.route.line.hasLogo || root.departure.route.line.hasModeLogo ? Kirigami.Units.largeSpacing : 0
            iconHeight: Kirigami.Units.iconSizes.medium
            source: root.departure.route.line.mode === KPublicTransport.Line.Unknown ? ReservationHelper.defaultIconName(root.reservation) : root.departure.route.line.iconName
            enabled: !root.controller.isCanceled
        }

        Kirigami.Heading {
            text: root.reservationFor.airline.iataCode + " " + root.reservationFor.flightNumber

            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.PlainText
            visible: !root.departure.route.line.hasLogo && (root.reservationFor.airline.iataCode|| root.reservationFor.flightNumber)
            enabled: !root.controller.isCanceled

            leftPadding: Kirigami.Units.smallSpacing
            rightPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing

            Layout.fillWidth: true
        }

        Kirigami.Heading {
            text: i18n("%1 to %2", root.airportDisplayString(root.reservationFor.departureAirport), root.airportDisplayString(root.reservationFor.arrivalAirport))

            level: 2
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.PlainText
            enabled: !root.controller.isCanceled

            leftPadding: Kirigami.Units.smallSpacing
            rightPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing

            Layout.fillWidth: true
        }

        FormCard.FormCard {
            visible: ticketToken.hasContent || sequenceNumberDelegate.visible

            // ticket barcode
            TicketTokenDelegate {
                id: ticketToken
                batchId: root.batchId
                onCurrentReservationIdChanged: {
                    if (!currentReservationId)
                        return;
                    root.currentReservationId = currentReservationId;
                }
                onScanModeToggled: scanModeController.toggle()
            }

            FormCard.FormDelegateSeparator {}

            // sequence number belongs to the selected barcode
            FormCard.FormTextDelegate {
                id: sequenceNumberDelegate
                text: i18n("Sequence Number:")
                description: root.reservation.passengerSequenceNumber
                visible: description
            }
        }

        // flight data
        FormCard.FormHeader {
            visible: boardingTimeLabel.visible || boardingGroupLabel.visible || seatLabel.visible || airlineNameLabel.visible
            title: i18n("Boarding")
        }

        FormCard.FormCard {
            visible: boardingTimeLabel.visible || boardingGroupLabel.visible || seatLabel.visible || airlineNameLabel.visible

            FormCard.FormTextDelegate {
                id: boardingTimeLabel
                visible: reservationFor.boardingTime > 0
                text: i18n("Boarding time")
                description: Localizer.formatDateTime(reservationFor, "boardingTime")
            }

            FormCard.FormDelegateSeparator { visible: boardingTimeLabel.visible }

            FormCard.FormTextDelegate {
                id: boardingGroupLabel
                visible: reservation.boardingGroup.length > 0
                text: i18n("Boarding group")
                description: reservation.boardingGroup
            }

            FormCard.FormDelegateSeparator { visible: boardingGroupLabel.visible }

            FormCard.FormTextDelegate {
                id: seatLabel
                text: i18nc("Flight seat", "Seat")
                description: reservation.airplaneSeat
                visible: reservation.airplaneSeat.length > 0
            }

            FormCard.FormDelegateSeparator { visible: seatLabel.visible }

            FormCard.FormTextDelegate {
                id: airlineNameLabel
                text: i18n("Airline")
                description: reservationFor.airline.name
                visible: reservationFor.airline.name.length > 0
            }
        }

        // departure data
        FormCard.FormHeader {
            title: i18nc("Flight departure", "Departure")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                id: departureTimeDelegate
                text: i18nc("flight departure", "Departure time")
                description: Localizer.formatDateTime(reservationFor, "departureTime")
                visible: reservationFor.departureTime > 0
            }
            FormCard.FormTextDelegate {
                text: i18nc("flight departure", "Departure date")
                visible: !departureTimeDelegate.visible && text.length > 0
                description: Localizer.formatDate(reservationFor, "departureDay")
            }

            FormCard.FormDelegateSeparator { visible: departureAirportDelegate.visible }

            FormPlaceDelegate {
                id: departureAirportDelegate

                text: i18n("Airport")
                placeName: airportDisplayString(reservationFor.departureAirport)
                visible: description.length > 0
                place: reservationFor.departureAirport
                controller: root.controller
                isRangeBegin: true
            }

            FormCard.FormDelegateSeparator { visible: departureTerminal.visible }

            FormCard.FormTextDelegate {
                id: departureTerminal

                text: i18n("Terminal")
                description: reservationFor.departureTerminal
                visible: reservationFor.departureTerminal.length > 0
            }

            FormCard.FormDelegateSeparator { visible: departureGate.visible }

            FormCard.FormTextDelegate {
                id: departureGate

                text: i18n("Gate")
                description: reservationFor.departureGate
                visible: reservationFor.departureGate.length > 0
            }

            FormCard.FormDelegateSeparator { visible: departureNotes.visible }
            FormCard.FormTextDelegate {
                id: departureNotes
                text: i18n("Additional information")
                description: root.controller.journey.notes.concat(root.departure.notes).join("<br/>")
                descriptionItem.textFormat: Text.RichText
                descriptionItem.wrapMode: Text.Wrap
                visible: description !== ""
                font.italic: true
                onLinkActivated: (link) => { Qt.openUrlExternally(link); }
            }
        }

        // arrival data
        FormCard.FormHeader {
            title: i18nc("Flight arrival", "Arrival")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                text: i18nc("flight arrival", "Arrival time")
                description: Localizer.formatDateTime(reservationFor, "arrivalTime")
                visible: reservationFor.arrivalTime > 0
            }

            FormCard.FormDelegateSeparator {
                visible: reservationFor.arrivalTime > 0 && arrivalAirportDelegate.visible
            }

            FormPlaceDelegate {
                id: arrivalAirportDelegate

                text: i18n("Airport")
                placeName: airportDisplayString(reservationFor.arrivalAirport)
                visible: text.length > 0
                place: reservationFor.arrivalAirport
                controller: root.controller
                isRangeEnd: true
            }

            FormCard.FormDelegateSeparator { visible: arrivalTerminal.visible }

            FormCard.FormTextDelegate {
                id: arrivalTerminal

                text: i18n("Terminal")
                description: reservationFor.arrivalTerminal
                visible: reservationFor.arrivalTerminal.length > 0
            }

            FormCard.FormDelegateSeparator { visible: arrivalNotes.visible }
            FormCard.FormTextDelegate {
                id: arrivalNotes
                text: i18n("Additional information")
                description: root.arrival.notes.join("<br/>")
                descriptionItem.textFormat: Text.RichText
                descriptionItem.wrapMode: Text.Wrap
                visible: description !== ""
                font.italic: true
                onLinkActivated: (link) => { Qt.openUrlExternally(link); }
            }
        }

        BookingCard {
            reservation: root.reservation
        }

        ProgramMembershipCard {
            programMembership: root.reservation.programMembershipUsed
        }

        ReservationDocumentsCard {
            controller: root.controller
        }

        ActionsCard {
            batchId: root.batchId
            reservationId: root.currentReservationId
            editor: root.editor
            reservation: root.reservation
            additionalActions: [
                QQC2.Action {
                    text: i18n("Alternatives")
                    icon.name: "clock"
                    onTriggered: applicationWindow().pageStack.push(Qt.createComponent('org.kde.itinerary', 'AlternativeJourneyPage'), {
                        controller: root.controller,
                        publicTransportManager: LiveDataManager.publicTransportManager
                    });
                }
            ]
        }

        // spacer for the floating buttons
        Item {
            visible: scanModeButton.visible
            implicitHeight: root.width < Kirigami.Units.gridUnit * 30 + scanModeButton.width * 2 ? scanModeButton.height : 0
        }
    }
}
