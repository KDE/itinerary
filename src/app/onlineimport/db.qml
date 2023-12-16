/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.itinerary

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
