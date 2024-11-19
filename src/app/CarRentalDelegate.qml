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
            Layout.fillWidth: true
            spacing: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

            TransportIcon {
                size: Kirigami.Units.iconSizes.smallMedium
                source: ReservationHelper.defaultIconName(root.reservation)
            }
            QQC2.Label {
                text: root.rangeType == TimelineElement.RangeEnd ?
                    i18n("Rental Car Drop-off") :
                    i18n("Rental Car Pick-up")
                color: root.headerTextColor
                Layout.fillWidth: true
            }
            QQC2.Label {
                text: root.rangeType ==  TimelineElement.RangeEnd ?
                    Localizer.formatTime(reservation, "dropoffTime") :
                    Localizer.formatTime(reservation, "pickupTime")
                color: root.headerTextColor
            }
        }

        QQC2.Label {
            Layout.fillWidth: true
            text: reservation.pickupLocation.name
            visible: root.rangeType != TimelineElement.RangeEnd
        }
        QQC2.Label {
            visible: text !== ""
            Layout.fillWidth: true
            text: Localizer.formatAddressWithContext(reservation.pickupLocation.address,
                                                     reservation.dropoffLocation.address,
                                                     Settings.homeCountryIsoCode)
        }
        QQC2.Label {
            Layout.fillWidth: true
            text: i18n("Drop-off: %1", Localizer.formatDateTime(reservation, "dropoffTime"))
            visible: root.rangeType != TimelineElement.RangeEnd
        }
        QQC2.Label {
            Layout.fillWidth: true
            text: reservation.dropoffLocation.name
            visible: root.rangeType != TimelineElement.RangeBegin
        }
        QQC2.Label {
            visible: text !== ""
            Layout.fillWidth: true
            text: Localizer.formatAddressWithContext(reservation.dropoffLocation.address,
                                                     reservation.pickupLocation.address,
                                                     Settings.homeCountryIsoCode)
        }
    }
}
