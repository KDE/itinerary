/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport
import org.kde.itinerary

TimelineDelegate {
    id: root

    contentItem: ColumnLayout {
        spacing: 0

        TimelineDelegateDepartureLayout {
            id: departureLayout
            reservationFor: root.reservationFor
            departure: root.departure
            progress: root.controller.progress
            departureName: reservationFor.departureBoatTerminal.name
            departureCountry: Localizer.formatAddressWithContext(reservationFor.departureBoatTerminal.address,

                                                                 reservationFor.arrivalBoatTerminal.address,
                                                                 Settings.homeCountryIsoCode)
            transportName: root.reservationFor.name.length > 0 ? root.reservationFor.name : i18nc("default transport name for a boat trip", "Ferry")
            transportIcon: root.departure.route.line.mode == Line.Unknown ? ReservationHelper.defaultIconName(root.reservation) : root.departure.route.line.iconName
        }

        TimelineDelegateArrivalLayout {
            depTimeWidth: departureLayout.depTimeWidth
            arrival: root.arrival
            progress: root.controller.progress
            reservationFor: root.reservationFor
            arrivalName: reservationFor.arrivalBoatTerminal.name
            arrivalCountry: Localizer.formatAddressWithContext(reservationFor.arrivalBoatTerminal.address,
                                                     reservationFor.departureBoatTerminal.address,
                                                     Settings.homeCountryIsoCode)
        }
    }
}

