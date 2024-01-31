/*
    SPDX-FileCopyrightText: 2021-2022 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.i18n.localeData
import org.kde.kirigami as Kirigami

/**
 * Combo box for showing a list of countries.
 * The model is expected to be an array of ISO 3166-1 alpha 2 codes.
 */
FormCard.FormComboBoxDelegate {
    id: controlRoot

    /** The currently selected country, as a KCountry object. */
    readonly property var currentCountry: Country.fromAlpha2(currentValue)

    /** Initially selected country. */
    property string initialCountry

    displayMode: Kirigami.Settings.isMobile ? FormCard.FormComboBoxDelegate.Page : FormCard.FormComboBoxDelegate.ComboBox

    displayText: currentCountry ? (currentCountry.emojiFlag + ' ' + currentCountry.name) : ""

    comboBoxDelegate: QQC2.ItemDelegate {
        id: delegate
        implicitWidth: ListView.view ? ListView.view.width : Kirigami.Units.gridUnit * 16
        highlighted: controlRoot.highlightedIndex === index
        property bool separatorVisible: false
        Kirigami.Theme.colorSet: controlRoot.Kirigami.Theme.inherit ? controlRoot.Kirigami.Theme.colorSet : Kirigami.Theme.View
        Kirigami.Theme.inherit: controlRoot.Kirigami.Theme.inherit
        text: {
            const c = Country.fromAlpha2(modelData);
            return c.emojiFlag + ' ' + c.name;
        }
        Accessible.name: Country.fromAlpha2(modelData).name
        Accessible.onPressAction: delegate.clicked()
    }

    dialogDelegate: QQC2.RadioDelegate {
        id: delegate
        implicitWidth: ListView.view ? ListView.view.width : Kirigami.Units.gridUnit * 16
        text: {
            const c = Country.fromAlpha2(modelData);
            return c.emojiFlag + ' ' + c.name;
        }
        checked: controlRoot.currentIndex === index
        property bool separatorVisible: false
        Kirigami.Theme.colorSet: controlRoot.Kirigami.Theme.inherit ? controlRoot.Kirigami.Theme.colorSet : Kirigami.Theme.View
        Kirigami.Theme.inherit: controlRoot.Kirigami.Theme.inherit
        onClicked: {
            controlRoot.currentIndex = index;
            controlRoot.activated(index);
            controlRoot.closeDialog();
        }
        Accessible.name: Country.fromAlpha2(modelData).name
        Accessible.onPressAction: delegate.clicked()
    }

    Component.onCompleted: if (initialCountry) {
        currentIndex = indexOfValue(initialCountry);
    }
}
