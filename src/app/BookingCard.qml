// SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import "." as App
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

MobileForm.FormCard {
    id: root

    required property var reservation

    visible: referenceLabel.visible || underNameLabel.visible || ticketNumberLabel.visible || priceLabel.visible

    Layout.topMargin: Kirigami.Units.largeSpacing
    Layout.fillWidth: true

    contentItem: ColumnLayout {
        // booking details
        MobileForm.FormCardHeader {
            title: i18n("Booking")
        }

        MobileForm.FormTextDelegate {
            id: referenceLabel
            text: i18n("Reference")
            description: root.reservation.reservationNumber
            visible: root.reservation.reservationNumber
        }

        MobileForm.FormDelegateSeparator { visible: referenceLabel.visible }

        MobileForm.FormTextDelegate {
            id: underNameLabel
            text: i18n("Under name")
            description: root.reservation.underName ? root.reservation.underName.name : ''
            visible: description
        }

        MobileForm.FormDelegateSeparator { visible: underNameLabel.visible }

        MobileForm.FormTextDelegate {
            id: ticketNumberLabel
            text: i18n("Ticket number:")
            description: root.reservation.reservedTicket ? root.reservation.reservedTicket.ticketNumber : ''
            visible: description.length > 0 && description !== referenceLabel.description
        }

        MobileForm.FormDelegateSeparator { visible: ticketNumberLabel.visible }

        FormPriceDelegate {
            id: priceLabel
            item: root.reservation
        }
    }
}
