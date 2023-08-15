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

    Layout.topMargin: Kirigami.Units.largeSpacing
    Layout.fillWidth: true

    contentItem: ColumnLayout {
        spacing: 0

        MobileForm.FormCardHeader {
            title: i18n("Booking")
        }

        MobileForm.FormTextFieldDelegate {
            id: referenceEdit
            label: i18n("Reference")
            text: item.reservationNumber
        }
        MobileForm.FormDelegateSeparator {}
        MobileForm.FormTextFieldDelegate {
            id: underNameEdit
            label: i18n("Under name")
            text: item.underName.name
        }
        MobileForm.FormDelegateSeparator {}
        App.FormPriceEditDelegate {
            id: priceEdit
            item: reservation
            defaultCurrency: Country.fromAlpha2(address.currentCountry).currencyCode
        }
    }
}
