/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.DetailsPage {
    id: root
    title: i18n("Rental Car")

    Kirigami.FormLayout {
        width: root.width

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Pick-up")
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Time:")
            text: Localizer.formatDateTime(reservation, "pickupTime")
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Location:")
            text: reservation.pickupLocation.name
        }
        App.PlaceDelegate {
            place: reservation.pickupLocation
            controller: root.controller
            isRangeBegin: true
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Drop-off")
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Time:")
            text: Localizer.formatDateTime(reservation, "pickupTime")
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Location:")
            text: reservation.dropoffLocation.name
        }
        App.PlaceDelegate {
            place: reservation.dropoffLocation
            controller: root.controller
            isRangeEnd: true
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Vehicle")
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Type:")
            text: reservationFor.name
            visible: reservationFor.name != ""
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Model:")
            text: reservationFor.model
            visible: reservationFor.model != ""
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Brand:")
            text: reservationFor.brand.name
            visible: reservationFor.brand.name != ""
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Reservation")
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Under name:")
            text: reservation.underName.name
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Reference:")
            text: reservation.reservationNumber
        }
    }
}

