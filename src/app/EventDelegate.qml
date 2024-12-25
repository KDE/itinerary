/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.itinerary

TimelineDelegate {
    id: root

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

            Layout.fillWidth: true

            Kirigami.Icon {
                Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                source: ReservationHelper.defaultIconName(root.reservation)
            }

            Kirigami.Heading {
                id: headerLabel

                level: 2
                text: root.rangeType == TimelineElement.RangeEnd ?
                    i18n("End: %1", reservationFor.name) : reservationFor.name
                elide: Text.ElideRight
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                Layout.fillWidth: true
                Accessible.ignored: true
            }

            Kirigami.Heading {
                level: 2
                text: {
                    if (root.rangeType == TimelineElement.RangeEnd)
                        return Localizer.formatTime(reservationFor, "endDate");
                    if (reservationFor.doorTime > 0)
                        return Localizer.formatTime(reservationFor, "doorTime");
                    return Localizer.formatTime(reservationFor, "startDate");
                }
            }
        }

        QQC2.Label {
            text: reservationFor.location != undefined ? reservationFor.location.name : ""
            visible: text !== ""
            wrapMode: Text.WordWrap

            Layout.fillWidth: true
        }
        QQC2.Label {
            visible: reservationFor.location != undefined && !reservationFor.location.address.isEmpty
            wrapMode: Text.WordWrap
            text: Localizer.formatAddressWithContext(reservationFor.location.address, null, Settings.homeCountryIsoCode)

            Layout.fillWidth: true
        }
        QQC2.Label {
            wrapMode: Text.WordWrap
            text: i18n("Start time: %1", Localizer.formatDateTime(reservationFor, "startDate"))
            visible: root.rangeType != TimelineElement.RangeEnd && reservationFor.doorTime > 0

            Layout.fillWidth: true
        }
        QQC2.Label {
            wrapMode: Text.WordWrap
            text: i18n("End time: %1", Localizer.formatDateTime(reservationFor, "endDate"));
            visible: root.rangeType != TimelineElement.RangeEnd && reservationFor.endDate > 0

            Layout.fillWidth: true
        }

        TimelineDelegateSeatRow {
            hasSeat: root.hasSeat
            lineSegmentVisible: false

            Layout.fillWidth: true

            TimelineDelegateSeatRowLabel {
                text: i18nc("event venue seat section", "Section: <b>%1</b>", reservation?.reservedTicket?.ticketedSeat?.seatSection || "-")
            }
            Kirigami.Separator {
                Layout.fillHeight: true
            }
            TimelineDelegateSeatRowLabel {
                text: i18nc("event venue seat row", "Row: <b>%1</b>", reservation?.reservedTicket?.ticketedSeat?.seatRow || "-")
            }
            Kirigami.Separator {
                Layout.fillHeight: true
            }
            TimelineDelegateSeatRowLabel {
                text: i18nc("event venue seat number", "Number: <b>%1</b>", reservation?.reservedTicket?.ticketedSeat?.seatNumber || "-")
            }
        }
    }

    Accessible.name: headerLabel.text
}
