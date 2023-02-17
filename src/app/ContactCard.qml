// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

MobileForm.FormCard {
    property var contact

    visible: contact.telephone || contact.email || contact.url != ""
    contentItem: ColumnLayout {
        spacing: 0

        MobileForm.FormCardHeader {
            title: i18n("Contact")
        }

        MobileForm.FormButtonDelegate {
            text: i18n("Telephone")
            description: contact.telephone
            icon.name: "call-start"
            onClicked: Qt.openUrlExternally(Util.telephoneUrl(contact.telephone))
            visible: contact.telephone
        }
        MobileForm.FormDelegateSeparator { visible: contact.telephone }

        MobileForm.FormButtonDelegate {
            text: i18n("Email")
            description: contact.email
            icon.name: "mail-message-new"
            onClicked: Qt.openUrlExternally(Util.emailUrl(contact.email))
            visible: contact.email
        }
        MobileForm.FormDelegateSeparator { visible: reservationFor.url != "" }

        MobileForm.FormButtonDelegate {
            text: i18n("Website")
            description: contact.url
            icon.name: "globe"
            onClicked: Qt.openUrlExternally(contact.url)
            visible: contact.url != ""
        }
    }
}
