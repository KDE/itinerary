// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitinerary
import org.kde.itinerary

ColumnLayout {
    id: root

    /** The object to edit. */
    property var item

    /** Default currency for the price edit field, if applicable. */
    property alias defaultCurrency: priceEdit.defaultCurrency

    /** Apply current edit state to @p item. */
    function apply(item) {
        item.reservationNumber = referenceEdit.text;

        let underName = item.underName;
        if (!underName)
            underName = Factory.makePerson();
        underName.name = underNameEdit.text;
        item.underName = underName;
        console.log(item.underName, item.underName.name);
        priceEdit.apply(item)
    }

    spacing: 0

    FormCard.FormHeader {
        title: i18nc("@title:group", "Booking")
    }

    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: referenceEdit
            label: i18n("Reference")
            text: root.item.reservationNumber
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextFieldDelegate {
            id: underNameEdit
            label: i18n("Under name")
            text: root.item.underName ? root.item.underName.name : ''
        }

        FormCard.FormDelegateSeparator { above: priceEdit }

        FormPriceEditDelegate {
            id: priceEdit
            item: root.item
        }
    }
}
