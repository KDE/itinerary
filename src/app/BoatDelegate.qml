/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport
import org.kde.itinerary

TimelineDelegate {
    id: root

    property bool expanded: false
    property KPublicTransport.journeySection journeySection: root.controller.journey

    JourneySectionModel {
        id: sectionModel
        journeySection: root.journeySection
        showProgress: root.controller.isCurrent
    }

    contentItem: ColumnLayout {
        spacing: 0

        TimelineDelegateDepartureLayout {
            id: departureLayout
            reservationFor: root.reservationFor
            departure: root.departure
            progress: root.controller.progress
            isCanceled: root.controller.isCanceled
            departureName: reservationFor.departureBoatTerminal.name
            departureCountry: Localizer.formatCountryWithContext(reservationFor.departureBoatTerminal.address,

                                                                 reservationFor.arrivalBoatTerminal.address,
                                                                 Settings.homeCountryIsoCode)
            transportName: root.reservationFor.name.length > 0 ? root.reservationFor.name : i18nc("default transport name for a boat trip", "Ferry")
            transportIcon: root.departure.route.line.mode == KPublicTransport.Line.Unknown ? ReservationHelper.defaultIconName(root.reservation) : root.departure.route.line.iconName

            TimelineDelegateIntermediateStopsButton {
                sectionModel: sectionModel
                expanded: root.expanded
                onToggled: root.expanded = !root.expanded
            }
        }

        TimelineDelegateIntermediateStopsView {
            sectionModel: sectionModel
            expanded: root.expanded
            isCanceled: root.controller.isCanceled
        }

        TimelineDelegateArrivalLayout {
            arrival: root.arrival
            progress: root.controller.progress
            reservationFor: root.reservationFor
            arrivalName: reservationFor.arrivalBoatTerminal.name
            isCanceled: root.controller.isCanceled
            arrivalCountry: Localizer.formatCountryWithContext(reservationFor.arrivalBoatTerminal.address,
                                                     reservationFor.departureBoatTerminal.address,
                                                     Settings.homeCountryIsoCode)
        }
    }
}

