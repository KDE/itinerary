// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitinerary
import org.kde.itinerary
import "." as App

ColumnLayout {
    id: root

    required property var contact

    spacing: 0
    visible: root.contact.telephone || root.contact.email || root.contact.url != ""

    FormCard.FormHeader {
        title: i18nc("@title:group", "Contact")
    }

    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            text: i18n("Telephone")
            description: contact.telephone
            icon.name: "call-start"
            onClicked: Qt.openUrlExternally(Util.telephoneUrl(contact.telephone))
            visible: contact.telephone
        }

        FormCard.FormDelegateSeparator { visible: contact.telephone }

        FormCard.FormButtonDelegate {
            text: i18n("Email")
            description: contact.email
            icon.name: "mail-message-new"
            onClicked: Qt.openUrlExternally(Util.emailUrl(contact.email))
            visible: contact.email
        }

        FormCard.FormDelegateSeparator { visible: reservationFor.url != "" }

        FormCard.FormButtonDelegate {
            text: i18n("Website")
            description: contact.url
            icon.name: "globe"
            onClicked: Qt.openUrlExternally(contact.url)
            visible: contact.url != ""
        }
    }
}
