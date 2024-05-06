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
    headerItem: QQC2.Label {
        id: headerLabel
        text: root.rangeType == TimelineElement.RangeEnd ?
            i18n("Check-out %1", reservationFor.name) : reservationFor.name
        color: root.headerTextColor
        elide: Text.ElideRight
        Layout.fillWidth: true
        Accessible.ignored: true
    }

    contentItem: Column {
        id: topLayout
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            visible: text !== ""
            width: topLayout.width
            text: Localizer.formatAddressWithContext(reservationFor.address, null, Settings.homeCountryIsoCode)
            wrapMode: Text.WordWrap
        }
        QQC2.Label {
            text: i18n("Check-in time: %1", Localizer.formatTime(reservation, "checkinTime"))
            color: Kirigami.Theme.textColor
            visible: root.rangeType == TimelineElement.RangeBegin && !Util.isStartOfDay(reservation, "checkinTime")
        }
        QQC2.Label {
            text: root.rangeType == TimelineElement.RangeBegin ?
                i18n("Check-out time: %1", !Util.isStartOfDay(reservation, "checkoutTime") ? Localizer.formatDateTime(reservation, "checkoutTime") : Localizer.formatDate(reservation, "checkoutTime")) :
                i18n("Check-out time: %1", Localizer.formatTime(reservation, "checkoutTime"))
            color: Kirigami.Theme.textColor
            visible: root.rangeType == TimelineElement.RangeBegin || !Util.isStartOfDay(reservation, "checkoutTime")
        }

    }

    onClicked: showDetailsPage(hotelDetailsPage, root.batchId)
    Accessible.name: headerLabel.text
}
