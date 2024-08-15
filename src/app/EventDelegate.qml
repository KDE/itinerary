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

    headerIconSource: ReservationHelper.defaultIconName(root.reservation)
    headerItem: RowLayout {
        QQC2.Label {
            id: headerLabel
            text: root.rangeType == TimelineElement.RangeEnd ?
                i18n("End: %1", reservationFor.name) : reservationFor.name
            color: root.headerTextColor
            elide: Text.ElideRight
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            Layout.fillWidth: true
            Accessible.ignored: true
        }
        QQC2.Label {
            text: {
                if (root.rangeType == TimelineElement.RangeEnd)
                    return Localizer.formatTime(reservationFor, "endDate");
                if (reservationFor.doorTime > 0)
                    return Localizer.formatTime(reservationFor, "doorTime");
                return Localizer.formatTime(reservationFor, "startDate");
            }
            color: root.headerTextColor
        }
    }

    contentItem: Column {
        id: topLayout
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            text: reservationFor.location != undefined ? reservationFor.location.name : ""
            visible: text !== ""
            wrapMode: Text.WordWrap
            width: topLayout.width
        }
        QQC2.Label {
            visible: reservationFor.location != undefined && !reservationFor.location.address.isEmpty
            width: topLayout.width
            text: Localizer.formatAddressWithContext(reservationFor.location.address, null, Settings.homeCountryIsoCode)
        }
        QQC2.Label {
            text: i18n("Start time: %1", Localizer.formatDateTime(reservationFor, "startDate"))
            visible: root.rangeType != TimelineElement.RangeEnd && reservationFor.doorTime > 0
        }
        QQC2.Label {
            text: i18n("End time: %1", Localizer.formatDateTime(reservationFor, "endDate"));
            visible: root.rangeType != TimelineElement.RangeEnd && reservationFor.endDate > 0
        }

        TimelineDelegateSeatRow {
            visible: root.hasSeat
            width: topLayout.width

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
