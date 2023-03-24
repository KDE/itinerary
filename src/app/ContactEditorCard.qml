// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
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
    id: root
    property var contact

    function save(contact) {
        contact.telephone = phoneEdit.text;
        contact.email = emailEdit.text;
        contact.url = urlEdit.text;
        return contact;
    }

    Layout.topMargin: Kirigami.Units.largeSpacing
    Layout.fillWidth: true

    contentItem: ColumnLayout {
        spacing: 0

        MobileForm.FormCardHeader {
            title: i18n("Contact")
        }

        MobileForm.FormTextFieldDelegate {
            id: phoneEdit
            label: i18n("Telephone")
            text: contact.telephone
            inputMethodHints: Qt.ImhDialableCharactersOnly
        }
        MobileForm.FormDelegateSeparator {}
        MobileForm.FormTextFieldDelegate {
            id: emailEdit
            label: i18n("Email")
            text: contact.email
            inputMethodHints: Qt.ImhEmailCharactersOnly
        }
        MobileForm.FormDelegateSeparator {}
        MobileForm.FormTextFieldDelegate {
            id: urlEdit
            label: i18n("Website")
            text: contact.url
            inputMethodHints: Qt.ImhUrlCharactersOnly
        }
    }
}
