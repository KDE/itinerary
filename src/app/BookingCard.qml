// SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.itinerary

/// Booking details
ColumnLayout {
    id: root

    required property var reservation

    visible: referenceLabel.visible || underNameLabel.visible || ticketNumberLabel.visible || priceLabel.visible

    Layout.fillWidth: true

    FormCard.FormHeader {
        title: i18n("Booking")
    }

    FormCard.FormCard {
        FormCard.FormTextDelegate {
            id: referenceLabel
            text: i18n("Reference")
            description: root.reservation.reservationNumber
            visible: root.reservation.reservationNumber

            trailing: QQC2.ToolButton {
                display: QQC2.AbstractButton.IconOnly
                text: i18nc("@info:tooltip", "Copy to Clipboard")
                icon.name: "edit-copy"
                onClicked: {
                    Clipboard.saveText(root.reservation.reservationNumber);
                    applicationWindow().showPassiveNotification(i18n("Booking reference copied to clipboard"));
                }

                QQC2.ToolTip.text: text
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.visible: hovered
            }
        }

        FormCard.FormDelegateSeparator { visible: referenceLabel.visible }

        FormCard.FormTextDelegate {
            id: underNameLabel
            text: i18n("Under name")
            description: root.reservation.underName ? root.reservation.underName.name : ''
            visible: description
        }

        FormCard.FormDelegateSeparator { visible: underNameLabel.visible }

        FormCard.FormTextDelegate {
            id: ticketNumberLabel
            text: i18n("Ticket number:")
            description: root.reservation.reservedTicket ? root.reservation.reservedTicket.ticketNumber : ''
            visible: description.length > 0 && description !== referenceLabel.description

            trailing: QQC2.ToolButton {
                display: QQC2.AbstractButton.IconOnly
                text: i18nc("@info:tooltip", "Copy to Clipboard")
                icon.name: "edit-copy"
                onClicked: {
                    Clipboard.saveText(root.reservation.reservedTicket.ticketNumber);
                    applicationWindow().showPassiveNotification(i18n("Ticket number copied to clipboard"));
                }

                QQC2.ToolTip.text: text
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.visible: hovered
            }
        }

        FormCard.FormDelegateSeparator { visible: ticketNumberLabel.visible }

        FormPriceDelegate {
            id: priceLabel
            item: root.reservation
        }
    }
}
