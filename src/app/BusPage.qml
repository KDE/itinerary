/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport
import org.kde.kpublictransport.ui as KPublicTransport
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
            text: root.reservationFor.busName + " " + root.reservationFor.busNumber
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.PlainText
            visible: !root.departure.route.line.hasLogo && (root.reservationFor.busName || root.reservationFor.busNumber)
            enabled: !root.controller.isCanceled

            leftPadding: Kirigami.Units.smallSpacing
            rightPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing

            Layout.fillWidth: true
        }

        Kirigami.Heading {
            text: i18n("%1 to %2", root.reservationFor.departureBusStop.name, root.reservationFor.arrivalBusStop.name)

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
            visible: ticketToken.hasContent
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

            FormCard.FormDelegateSeparator { visible: stationName.visible }

            FormPlaceDelegate {
                id: stationName

                text: i18nc("bus station", "Station")
                description: reservationFor.departureBusStop.name
                place: reservationFor.departureBusStop
                controller: root.controller
                isRangeBegin: true
                visible: visible
            }

            FormCard.FormDelegateSeparator { visible: departurePlatformDelegate.visible }

            FormCard.FormTextDelegate {
                id: departurePlatformDelegate
                text: i18nc("bus station platform", "Platform")
                description: reservationFor.departurePlatform
                visible: description !== ""
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

            FormCard.FormDelegateSeparator { visible: reservationFor.arrivalTime > 0 && arrivalDelegate.visible }

            FormPlaceDelegate {
                id: arrivalDelegate

                text: i18nc("bus station", "Station")
                place: reservationFor.arrivalBusStop
                controller: root.controller
                placeName: reservationFor.arrivalBusStop.name
                isRangeEnd: true
            }

            FormCard.FormDelegateSeparator { visible: arrivalPlatformDelegate.visible }

            FormCard.FormTextDelegate {
                id: arrivalPlatformDelegate
                text: i18nc("bus station platform", "Platform")
                description: reservationFor.arrivalPlatform
                visible: description !== ""
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

        // seat reservation
        FormCard.FormHeader {
            title: i18nc("Bus seat", "Seat")
            visible: seatCard.visible
        }

        FormCard.FormCard {
            id: seatCard
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
            reservationId: root.currentReservationId
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
                    onTriggered: applicationWindow().pageStack.push(journeySectionPage, {
                        journeySection: root.controller.trip,
                        departureStopIndex: root.controller.tripDepartureIndex,
                        arrivalStopIndex: root.controller.tripArrivalIndex,
                        showProgress: root.controller.isCurrent
                    });
                    Component.onCompleted: {
                        visible = Qt.binding(function() { return root.controller.journey && (root.controller.journey.intermediateStops.length > 0 || !root.controller.journey.path.isEmpty); });
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

        // spacer for the floating buttons
        Item {
            visible: scanModeButton.visible
            implicitHeight: root.width < Kirigami.Units.gridUnit * 30 + scanModeButton.width * 2 ? scanModeButton.height : 0
        }
    }
}
