// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kpublictransport 1.0 as KPublicTransport
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.DetailsPage {
    id: root
    title: i18n("Train Ticket")
    editor: App.TrainEditor {
        controller: root.controller
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

            property bool arrival

            onLayoutUpdated: root.controller.setVehicleLayout(vehicleLayout, arrival);
        }
    }

    BarcodeScanModeController {
        id: scanModeController
        page: root
    }

    ColumnLayout {
        width: parent.width

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                Kirigami.Heading {
                    Layout.fillWidth: true
                    Layout.topMargin: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing
                    text: {
                        if (reservationFor.trainName || reservationFor.trainNumber) {
                            return reservationFor.trainName + " " + reservationFor.trainNumber
                        }
                        return i18n("%1 to %2", reservationFor.departureStation.name, reservationFor.arrivalStation.name);
                    }
                    horizontalAlignment: Qt.AlignHCenter
                    font.bold: true
                }

                // ticket barcode
                App.TicketTokenDelegate {
                    id: ticketToken
                    Layout.fillWidth: true
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
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                // departure data
                MobileForm.FormCardHeader {
                    title: i18n("Departure")
                }
                MobileForm.FormTextDelegate {
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
                MobileForm.FormTextDelegate {
                    text: i18n("Departure date")
                    visible: !departureTimeDelegate.visible && text.length > 0
                    description: Localizer.formatDate(reservationFor, "departureDay")
                }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormTextDelegate {
                    text: i18nc("train station", "Station")
                    description: reservationFor.departureStation.name
                    visible: description
                }

                MobileForm.FormDelegateSeparator { visible: reservationFor.departureStation.name }

                App.FormPlatformDelegate {
                    id: departurePlatformDelegate
                    stopover: departure
                    sections: root.controller.departurePlatformSections
                    scheduledPlatform: reservationFor.departurePlatform
                }

                MobileForm.FormDelegateSeparator { visible: departurePlatformDelegate.visible }

                App.FormPlaceDelegate {
                    id: departureDelegate
                    place: reservationFor.departureStation
                    controller: root.controller
                    isRangeBegin: true
                }

                MobileForm.FormDelegateSeparator {
                    visible: departureDelegate.visible
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Additional notes")
                    description: departure.notes.join("<br/>")
                    descriptionItem.textFormat: Text.RichText
                    descriptionItem.wrapMode: Text.Wrap
                    visible: arrival.notes.length > 0
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                // arrival data
                MobileForm.FormCardHeader {
                    title: i18n("Arrival")
                }

                MobileForm.FormTextDelegate {
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

                MobileForm.FormDelegateSeparator { visible: arrivalTimeLabel.text.length > 0 }

                MobileForm.FormTextDelegate {
                    text: i18nc("train station", "Station")
                    description: reservationFor.arrivalStation.name
                }

                MobileForm.FormDelegateSeparator { visible: reservationFor.arrivalStation.name }

                App.FormPlatformDelegate {
                    id: arrivalPlatformDelegate
                    stopover: arrival
                    sections: root.controller.arrivalPlatformSections
                    scheduledPlatform: reservationFor.arrivalPlatform
                }

                MobileForm.FormDelegateSeparator { visible: arrivalPlatformDelegate.visible }

                App.FormPlaceDelegate {
                    id: arrivalDelegate
                    place: reservationFor.arrivalStation
                    controller: root.controller
                    isRangeEnd: true
                }

                MobileForm.FormDelegateSeparator {
                    visible: arrivalDelegate
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Additional notes")
                    description: arrival.notes.join("<br/>")
                    descriptionItem.textFormat: Text.RichText
                    descriptionItem.wrapMode: Text.Wrap
                    visible: arrival.notes.length > 0
                    font.italic: true
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }
        }

        // seat reservation
        MobileForm.FormCard {
            visible: coachLabel.visible || seatLabel.visible || classLabel.visible
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                MobileForm.FormCardHeader {
                    title: i18n("Seat")
                }
                MobileForm.FormTextDelegate {
                    id: coachLabel
                    text: i18nc("coach of a train", "Coach:")
                    description: currentReservation.reservedTicket ? currentReservation.reservedTicket.ticketedSeat.seatSection : ''
                    visible: description
                }
                MobileForm.FormDelegateSeparator {
                    visible: currentReservation.reservedTicket ? currentReservation.reservedTicket.ticketedSeat.seatSection : false
                }
                MobileForm.FormTextDelegate {
                    id: seatLabel
                    text: i18n("Seat:")
                    description: currentReservation.reservedTicket ? currentReservation.reservedTicket.ticketedSeat.seatNumber : ''
                    visible: description
                }
                MobileForm.FormDelegateSeparator {
                    visible: currentReservation.reservedTicket ? currentReservation.reservedTicket.ticketedSeat.seatNumber : false
                }
                MobileForm.FormTextDelegate {
                    id: classLabel
                    text: i18n("Class:")
                    description: currentReservation.reservedTicket ? currentReservation.reservedTicket.ticketedSeat.seatingType : ''
                    visible: description
                }
            }
        }

        // program membership
        MobileForm.FormCard {
            visible: programNameLabel.visible || membershipNumberLabel.visible
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                MobileForm.FormCardHeader {
                    title: i18nc("bonus, discount or frequent traveler program", "Program")
                }
                MobileForm.FormTextDelegate {
                    id: programNameLabel
                    text: i18n("Name")
                    description: root.currentReservation.programMembershipUsed.programName
                    visible: description
                }
                MobileForm.FormDelegateSeparator {
                    visible: programNameLabel.visible
                }
                MobileForm.FormTextDelegate {
                    id: membershipNumberLabel
                    text: i18n("Number")
                    description: root.currentReservation.programMembershipUsed.membershipNumber
                    visible: description
                }
                MobileForm.FormDelegateSeparator {
                    visible: membershipNumberLabel.visible
                }
                MobileForm.FormButtonDelegate {
                    id: passButton
                    text: i18n("Show program pass")
                    property string passId: PassManager.findMatchingPass(root.currentReservation.programMembershipUsed)
                    visible: passId
                    onClicked: applicationWindow().pageStack.push(programMembershipPage, {
                        programMembership: PassManager.pass(passButton.passId),
                        passId: passButton.passId,
                    })
                }
            }
        }

        App.BookingCard {
            currentReservation: root.currentReservation
            reservation: root.reservation
        }

        App.DocumentsPage {
            controller: root.controller
        }

        App.ActionsCard {
            batchId: root.batchId
            editor: root.editor
            additionalActions: [
                QQC2.Action {
                    text: i18n("Alternatives")
                    icon.name: "clock"
                    onTriggered: applicationWindow().pageStack.push(alternativePage)
                },
                QQC2.Action {
                    text: i18n("Departure Vehicle Layout")
                    icon.name: "view-list-symbolic"
                    enabled: departure && departure.route.line.mode == KPublicTransport.Line.LongDistanceTrain
                    onTriggered: applicationWindow().pageStack.push(vehicleLayoutPage, {stopover: root.controller.departure, arrival: false})
                },
                QQC2.Action {
                    text: i18n("Arrival Vehicle Layout")
                    icon.name: "view-list-symbolic"
                    enabled: arrival && arrival.route.line.mode == KPublicTransport.Line.LongDistanceTrain
                    onTriggered: applicationWindow().pageStack.push(vehicleLayoutPage, {stopover: root.controller.arrival, arrival: true});
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
