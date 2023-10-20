// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.i18n.localeData 1.0
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

FormCard.AbstractFormDelegate {
    id: root

    /** The element of which the price properties should be edited. */
    property var item
    /** Default currency if no price is already set on @p item. */
    property string defaultCurrency: Country.fromAlpha2(Settings.homeCountryIsoCode).currencyCode

    /** Save the current value/currency to @p item. */
    function apply(item)
    {
        // TODO deal with the price being set on the nested ticket
        item.totalPrice = Number.fromLocaleString(Qt.locale(), priceEdit.text);
        item.priceCurrency = currencyBox.currentText;
    }

    // input validation, as in FormTextFieldDelegate
    property alias status: formErrorHandler.type
    property alias statusMessage: formErrorHandler.text

    text: i18n("Price")
    visible: PriceUtil.canHavePrice(root.item)

    focusPolicy: Qt.NoFocus
    onActiveFocusChanged: {
        if (activeFocus) {
            priceEdit.forceActiveFocus();
        }
    }
    onClicked: priceEdit.forceActiveFocus()
    background: null

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing
        QQC2.Label {
            Layout.fillWidth: true
            elide: Text.ElideRight
            text: root.text
            Accessible.ignored: true
        }

        RowLayout {
            // place currency before or after the value depending on the current locale
            layoutDirection: Number(1).toLocaleCurrencyString(Qt.locale(), 'X').startsWith('X') ? Qt.RightToLeft : Qt.LeftToRight

            QQC2.TextField {
                id: priceEdit

                text: PriceUtil.hasPrice(root.item) ? Number(PriceUtil.price(root.item)).toLocaleString(Qt.locale(), 'f', priceValidator.decimals) : ''

                inputMethodHints: Qt.ImhFormattedNumbersOnly
                validator: DoubleValidator {
                    id: priceValidator
                    bottom: 0
                    decimals: PriceUtil.decimalCount(currencyBox.currentText)
                    notation: DoubleValidator.StandardNotation
                }
            }

            QQC2.ComboBox {
                id: currencyBox
                function unique(a) { return [...new Set(a)]; }
                model: unique(Country.allCountries.map(c => c.currencyCode).filter((code) => code != '').sort())
                Component.onCompleted: {
                    currencyBox.currentIndex = Qt.binding(function() { return currencyBox.find(PriceUtil.hasPrice(root.item) ? PriceUtil.currency(root.item) : root.defaultCurrency, Qt.MatchExactly) });
                }
            }
        }
        Kirigami.InlineMessage {
            id: formErrorHandler
            visible: formErrorHandler.text.length > 0
            Layout.topMargin: visible ? Kirigami.Units.smallSpacing : 0
            Layout.fillWidth: true
            type: Kirigami.MessageType.Warning
            text: {
                try {
                    Number.fromLocaleString(Qt.locale(), priceEdit.text);
                } catch(err) {
                    return i18n("Not a valid number.");
                }
                return "";
            }
        }
    }
}
