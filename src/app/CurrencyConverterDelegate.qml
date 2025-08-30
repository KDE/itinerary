// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.itinerary

FormCard.AbstractFormDelegate {
    id: delegate

    required property string homeCurrency
    required property string foreignCurrency

    property real initialPrice: {
        // Choose an initial price that doesn't result in a foreign price of 0.
        const minConvertedPrice = Math.pow(10, -homePriceValidator.decimals);
        let initialPrice = 1.0;
        while (UnitConversion.convertCurrency(initialPrice, foreignCurrency, homeCurrency) < minConvertedPrice) {
             initialPrice *= 10;
        }
        return initialPrice;
    }

    // Offer converter only if conversion is possible.
    readonly property bool valid: !isNaN(UnitConversion.convertCurrency(initialPrice, foreignCurrency, homeCurrency))

    Accessible.name: i18n("Currency: %1", delegate.foreignCurrency)
    icon.name: "view-currency-list"
    background: null

    function priceFromText(text : string) : double {
        try {
            return Number.fromLocaleString(Qt.locale(), text);
        } catch (e) {
            return NaN;
        }
    }

    contentItem: RowLayout {
        Kirigami.Icon {
            source: delegate.icon.name
            implicitWidth: Kirigami.Units.iconSizes.smallMedium
            implicitHeight: Kirigami.Units.iconSizes.smallMedium
        }

        ColumnLayout {
            Layout.fillWidth: true

            QQC2.Label {
                Layout.fillWidth: true
                text: delegate.Accessible.name
            }

            RowLayout {
                Layout.fillWidth: true
                visible: delegate.valid

                QQC2.TextField {
                    id: foreignCurrencyEdit
                    property bool textEditRecursionGuard: false
                    Layout.fillWidth: true
                    horizontalAlignment: QQC2.TextField.AlignRight
                    inputMethodHints: Qt.ImhFormattedNumbersOnly

                    text: delegate.initialPrice.toLocaleString(Qt.locale(), 'f', foreignPriceValidator.decimals)

                    validator: DoubleValidator {
                        id: foreignPriceValidator
                        bottom: 0
                        decimals: PriceUtil.decimalCount(delegate.foreignCurrency)
                        notation: DoubleValidator.StandardNotation
                    }

                    onActiveFocusChanged: {
                        if (activeFocus) {
                            selectAll();
                        } else {
                            const price = delegate.priceFromText(text);
                            if (!isNaN(price)) {
                                text = price.toLocaleString(Qt.locale(), 'f', foreignPriceValidator.decimals);
                            }
                        }
                    }

                    // TODO just use onTextEdited once we can depend on Qt 6.9.
                    onTextChanged: updateHomePrice()
                    Component.onCompleted: {
                        if (Settings.performCurrencyConversion) {
                            updateHomePrice();
                        }
                    }

                    function updateHomePrice() : void {
                        if (textEditRecursionGuard) {
                            return;
                        }

                        const price = delegate.priceFromText(text);
                        const homePrice = UnitConversion.convertCurrency(price, delegate.foreignCurrency, delegate.homeCurrency);
                        if (!isNaN(homePrice)) {
                            homeCurrencyEdit.textEditRecursionGuard = true;
                            homeCurrencyEdit.text = homePrice.toLocaleString(Qt.locale(), 'f', homePriceValidator.decimals);
                            homeCurrencyEdit.textEditRecursionGuard = false;
                        }
                    }
                }

                QQC2.Label {
                    text: delegate.foreignCurrency
                }

                QQC2.Label {
                    text: "="
                }

                QQC2.TextField {
                    id: homeCurrencyEdit
                    property bool textEditRecursionGuard: false
                    Layout.fillWidth: true
                    horizontalAlignment: QQC2.TextField.AlignRight
                    inputMethodHints: Qt.ImhFormattedNumbersOnly

                    validator: DoubleValidator {
                        id: homePriceValidator
                        bottom: 0
                        decimals: PriceUtil.decimalCount(delegate.homeCurrency)
                        notation: DoubleValidator.StandardNotation
                    }

                    onActiveFocusChanged: {
                        if (activeFocus) {
                            selectAll();
                        } else {
                            const price = delegate.priceFromText(text);
                            if (!isNaN(price)) {
                                text = price.toLocaleString(Qt.locale(), 'f', homePriceValidator.decimals);
                            }
                        }
                    }

                    // TODO just use onTextEdited once we can depend on Qt 6.9.
                    onTextChanged: {
                        if (!textEditRecursionGuard) {
                            const price = delegate.priceFromText(text);
                            const foreignPrice = UnitConversion.convertCurrency(price, delegate.homeCurrency, delegate.foreignCurrency);
                            if (!isNaN(foreignPrice)) {
                                foreignCurrencyEdit.textEditRecursionGuard = true;
                                foreignCurrencyEdit.text = foreignPrice.toLocaleString(Qt.locale(), 'f', foreignPriceValidator.decimals);
                                foreignCurrencyEdit.textEditRecursionGuard = false;
                            }
                        }
                    }
                }

                QQC2.Label {
                    text: delegate.homeCurrency
                }
            }
        }
    }
}
