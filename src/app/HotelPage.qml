// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.coreaddons as KCoreAddons
import org.kde.kitinerary
import org.kde.itinerary

ScrollableDetailsPage {
    id: root

    function formatStayDate(propertyName: string): string {
        const date = Util.dateTimeStripTimezone(root.reservation, propertyName);
        if (isNaN(date.getTime()))
            return "";
        const day = Qt.formatDate(date, Qt.locale().dateFormat(Locale.LongFormat));
        return Util.isStartOfDay(root.reservation, propertyName) ? day
            : i18nc("date and time of a hotel stay", "%1 · %2", day, KCoreAddons.Format.formatTime(root.reservation, propertyName, KCoreAddons.FormatTypes.AddTimezoneAbbreviationIfNeeded));
    }

    title: i18n("Hotel Reservation")

    editor: "HotelEditor"

    data: BarcodeScanModeButton {
        id: scanModeButton
        page: root
        visible: ticketToken.hasBarcode
    }

    ColumnLayout {
        spacing: 0

        CardPageTitle {
            emojiIcon: "🏨"
            text: root.reservationFor.name
            enabled: !root.controller.isCanceled
        }

        FormCard.FormCard {
            visible: reservation.checkinTime > 0 || reservation.checkoutTime > 0
            autoSeparators: true

            Layout.topMargin: Kirigami.Units.largeSpacing

            FormCard.FormTextDelegate {
                text: i18n("Check-in time")
                visible: reservation.checkinTime > 0
                description: root.formatStayDate("checkinTime")
            }

            FormCard.FormTextDelegate {
                text: i18n("Check-out time")
                visible: reservation.checkoutTime > 0
                description: root.formatStayDate("checkoutTime")
            }
        }

        FormCard.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing

            visible: ticketToken.hasContent
            // ticket barcode
            TicketTokenDelegate {
                id: ticketToken
                Layout.fillWidth: true
                batchId: root.batchId
                onCurrentReservationIdChanged: {
                    if (!currentReservationId)
                        return;
                    root.currentReservationId = currentReservationId;
                }
                onScanModeToggled: scanModeButton.toggle()
            }
        }

        FormCard.FormHeader {
            title: i18n("Location")
        }

        FormCard.FormCard {
            FormPlaceDelegate {
                place: reservationFor
                controller: root.controller
            }
        }

        ContactCard {
            contact: root.reservationFor
        }

        FormCard.FormCard {
            visible: reservation.lodgingUnitDescription !== ""
            FormCard.FormTextDelegate {
                text: i18n("Description")
                description: reservation.lodgingUnitDescription
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
            controller: root.controller
            batchId: root.batchId
            reservationId: root.currentReservationId
            editor: root.editor
            reservation: root.reservation
        }
    }
}
