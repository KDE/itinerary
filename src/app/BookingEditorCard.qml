// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

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

        App.FormPriceEditDelegate {
            id: priceEdit
            item: root.item
        }
    }
}
