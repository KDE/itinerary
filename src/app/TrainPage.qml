/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.17 as Kirigami
import org.kde.kpublictransport 1.0 as KPublicTransport
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
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

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                 spacing: 0
                 Kirigami.Heading {
                     Layout.fillWidth: true
                     Layout.topMargin: Kirigami.Units.largeSpacing
                     Layout.bottomMargin: Kirigami.Units.largeSpacing
                     text: reservationFor.trainName + " " + reservationFor.trainNumber
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
                 }

                // departure data
                MobileForm.FormCardHeader {
                    title: i18n("Departure")
                }
                MobileForm.FormTextDelegate {
                    visible: departureTimeLabel.length > 0
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
                                font: Kirigami.Theme.smallFont
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

                MobileForm.FormTextDelegate {
                    text: i18nc("train station", "Station")
                    description: reservationFor.departureStation.name
                }

                MobileForm.FormTextDelegate {
                    contentItem: App.PlaceDelegate {
                        place: reservationFor.departureStation
                        controller: root.controller
                        isRangeBegin: true
                    }
                }

                MobileForm.FormTextDelegate {
                    visible: departurePlatformLabel.text != ""
                    contentItem: ColumnLayout {
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing
                        QQC2.Label {
                            Layout.fillWidth: true
                            text: i18n("Platform")
                            elide: Text.ElideRight
                        }
                        RowLayout {
                            QQC2.Label {
                                Layout.fillWidth: true
                                id: departurePlatformLabel
                                font: Kirigami.Theme.smallFont
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
                    }
                }
                MobileForm.FormTextDelegate {
                    visible: departure.notes.length > 0
                    text: departure.notes.join("<br/>")
                    onLinkActivated: Qt.openUrlExternally(link)
                }

                // arrival data
                MobileForm.FormCardHeader {
                    title: i18n("Arrival")
                }

                MobileForm.FormTextDelegate {
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
                            }
                        }
                    }
                }
                MobileForm.FormTextDelegate {
                    text: i18nc("train station", "Station")
                    description: reservationFor.arrivalStation.name
                }

                MobileForm.FormTextDelegate {
                    contentItem: App.PlaceDelegate {
                        place: reservationFor.arrivalStation
                        controller: root.controller
                        isRangeBegin: true
                    }
                }

                RowLayout {
                    Kirigami.FormData.label: i18n("Platform:")
                    visible: arrivalPlatformLabel.text.length > 0
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
                    description: currentReservation.reservedTicket.ticketedSeat.seatSection
                    visible: description
                }
                MobileForm.FormTextDelegate {
                    id: seatLabel
                    text: i18n("Seat:")
                    description: currentReservation.reservedTicket.ticketedSeat.seatNumber
                    visible: description
                }
                MobileForm.FormTextDelegate {
                    id: classLabel
                    text: i18n("Class:")
                    description: currentReservation.reservedTicket.ticketedSeat.seatingType
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
                MobileForm.FormTextDelegate {
                    id: membershipNumberLabel
                    text: i18n("Number")
                    description: root.currentReservation.programMembershipUsed.membershipNumber
                    visible: description
                }
                MobileForm.FormButtonDelegate {
                    id: passButton
                    text: i18n("Show program pass")
                    property string passId: PassManager.findMatchingPass(root.currentReservation.programMembershipUsed)
                    visible: passId
                    onClicked: applicationWindow().pageStack.push(programMembershipPage, { programMembership: PassManager.pass(passButton.passId), passId: passButton.passId })
                }
            }
        }

        MobileForm.FormCard {
            visible: referenceLabel.visible || underNameLabel.visible
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                // booking details
                MobileForm.FormCardHeader {
                    title: i18n("Booking")
                }
                MobileForm.FormTextDelegate {
                    id: referenceLabel
                    text: i18n("Reference:")
                    description: reservation.reservationNumber
                    visible: reservation.reservationNumber
                }
                MobileForm.FormTextDelegate {
                    id: underNameLabel
                    text: i18n("Under name:")
                    description: reservation.underName.name
                    visible: reservation.underName.name !== ""
                }
            }
        }

        App.DocumentsPage {
            controller: root.controller
        }

        App.ActionsCard {
            batchId: root.batchId
            editor: root.editor
        }
    }
}
