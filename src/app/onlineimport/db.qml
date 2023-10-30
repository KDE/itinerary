/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.itinerary 1.0

ColumnLayout {
    id: root

    property var arguments: {
        if (nameInput.text === "" || (bookingReferenceInput.text.length !== 6 && bookingReferenceInput.text.length !== 12)) {
            return undefined;
        }
        return { name: nameInput.text, reference: bookingReferenceInput.text };
    }

    signal search()

    FormCard.FormTextFieldDelegate {
        id: nameInput
        label: i18n("Family name")
        // TODO can we prefill this with the user name
        text: Settings.read("OnlineImport/Name", "")
        onEditingFinished: Settings.write("OnlineImport/Name", nameInput.text)
        status: Kirigami.MessageType.Information
        statusMessage: (text.length > 2 && (text.toUpperCase() == text || text.toLowerCase() == text)) ? i18n("Name is case-sensitive.") : ""
    }

    FormCard.FormDelegateSeparator {}

    FormCard.FormTextFieldDelegate {
        id: bookingReferenceInput
        label: i18n("Order number or booking reference")
        placeholderText: "123456789123"
        onAccepted: root.search()
    }

    Component.onCompleted: {
        if (nameInput.text === "") {
            nameInput.forceActiveFocus();
        } else {
            bookingReferenceInput.forceActiveFocus();
        }
    }
}
