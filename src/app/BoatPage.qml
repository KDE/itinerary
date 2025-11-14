// SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitinerary
import org.kde.itinerary

DetailsPage {
    id: root

    title: i18n("Boat Ticket")

    editor: BoatEditor {
        controller: root.controller
    }

    data: BarcodeScanModeButton {
        page: root
        visible: ticketToken.hasBarcode
    }

    ColumnLayout {
        spacing: 0

        CardPageTitle {
            emojiIcon: "🛳️"
            text: root.reservationFor.name.length > 0 ? root.reservationFor.name : i18nc("default transport name for a boat trip", "Ferry")

            // TODO vessel name not yet available in the data model
            // text: reservationFor.boatName
        }

        FormCard.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            visible: ticketToken.hasContent

            // ticket barcode
            TicketTokenDelegate {
                id: ticketToken
                Layout.topMargin: Kirigami.Units.largeSpacing
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
            title: i18nc("Boat departure", "Departure")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                text: i18n("Departure time")
                description: Localizer.formatDateTime(reservationFor, "departureTime")
            }

            FormCard.FormDelegateSeparator { visible: departureTerminal.visible }

            FormPlaceDelegate {
                id: departureTerminal

                text: i18nc("Boat terminal", "Terminal")
                placeName: reservationFor.departureBoatTerminal.name
                place: reservationFor.departureBoatTerminal
                controller: root.controller
                isRangeBegin: true
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
            title: i18nc("Boat arrival", "Arrival")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                text: i18n("Arrival time")
                description: Localizer.formatDateTime(reservationFor, "arrivalTime")
            }

            FormCard.FormDelegateSeparator { visible: arrivalTerminal.visible }

            FormPlaceDelegate {
                id: arrivalTerminal

                place: reservationFor.arrivalBoatTerminal
                controller: root.controller
                isRangeEnd: true
                text: i18nc("Boat terminal", "Terminal")
                placeName: reservationFor.arrivalBoatTerminal.name
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
                    onTriggered: applicationWindow().pageStack.push(Qt.createComponent('org.kde.itinerary', 'AlternativeJourneyPage'), {
                        controller: root.controller,
                        publicTransportManager: LiveDataManager.publicTransportManager
                    });
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
        }
    }
}

