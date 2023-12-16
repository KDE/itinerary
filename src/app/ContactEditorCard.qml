// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
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

    Layout.fillWidth: true

    function save(contact) {
        contact.telephone = phoneEdit.text;
        contact.email = emailEdit.text;
        contact.url = urlEdit.text;
        return contact;
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Contact")
    }

    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: phoneEdit
            label: i18n("Telephone")
            text: root.contact.telephone
            inputMethodHints: Qt.ImhDialableCharactersOnly
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextFieldDelegate {
            id: emailEdit
            label: i18n("Email")
            text: root.contact.email
            inputMethodHints: Qt.ImhEmailCharactersOnly
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextFieldDelegate {
            id: urlEdit
            label: i18n("Website")
            text: root.contact.url
            inputMethodHints: Qt.ImhUrlCharactersOnly
        }
    }
}
