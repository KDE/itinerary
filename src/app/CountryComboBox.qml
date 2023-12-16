/*
    SPDX-FileCopyrightText: 2021-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.i18n.localeData

/** Combo box for showing a list of countries.
 *  The model is expected to be an array of ISO 3166-1 alpha 2 codes.
 */
QQC2.ComboBox {
    /** The currently selected country, as a KCountry object. */
    readonly property var currentCountry: Country.fromAlpha2(currentValue)

    /** Initially selected country. */
    property string initialCountry

   onModelChanged: {
       model = model.sort((lhs, rhs) => { return Country.fromAlpha2(lhs).name.localeCompare(Country.fromAlpha2(rhs).name); });
   }

    displayText: currentCountry ? (currentCountry.emojiFlag + ' ' + currentCountry.name) : ""

    delegate: QQC2.ItemDelegate {
        text: {
            const c = Country.fromAlpha2(modelData);
            return c.emojiFlag + ' ' + c.name;
        }
        width: parent ? parent.width : undefined
    }

    Component.onCompleted: {
        if (initialCountry) {
            currentIndex = indexOfValue(initialCountry);
        }
    }
}
