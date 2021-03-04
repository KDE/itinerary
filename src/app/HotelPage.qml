/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.12 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.DetailsPage {
    id: root
    title: i18n("Hotel Reservation")
    editor: Component {
        App.HotelEditor {
            batchId: root.batchId
        }
    }

    Kirigami.FormLayout {
        width: root.width

        QQC2.Label {
            Kirigami.FormData.isSection: true
            text: reservationFor.name
            horizontalAlignment: Qt.AlignHCenter
            font.bold: true
        }

        App.PlaceDelegate {
            Kirigami.FormData.label: i18n("Location:")
            Kirigami.FormData.labelAlignment: Qt.AlignTop
            place: reservationFor
            controller: root.controller
        }

        QQC2.Label {
            Kirigami.FormData.label: i18n("Telephone:")
            text: Util.textToHtml(reservationFor.telephone)
            onLinkActivated: Qt.openUrlExternally(link)
            visible: reservationFor.telephone
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Email:")
            text: Util.textToHtml(reservationFor.email)
            onLinkActivated: Qt.openUrlExternally(link)
            visible: reservationFor.email
        }

        QQC2.Label {
            Kirigami.FormData.label: i18n("Check-in time:")
            text: Localizer.formatDateTime(reservation, "checkinTime")
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Check-out time:")
            text: Localizer.formatDateTime(reservation, "checkoutTime")
        }

        QQC2.Label {
            Kirigami.FormData.label: i18n("Booking reference:")
            text: reservation.reservationNumber
            visible: reservation.reservationNumber != ""
        }
        QQC2.Label {
            Kirigami.FormData.label: i18n("Under name:")
            text: reservation.underName.name
            visible: reservation.underName.name != ""
        }
    }
}
