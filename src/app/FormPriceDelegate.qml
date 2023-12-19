// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.i18n.localeData
import org.kde.kitinerary
import org.kde.itinerary

FormCard.FormTextDelegate {
    id: root

    /** The element of which the price properties should be shown. */
    required property var item

    text: i18nc("@label", "Price")
    visible: PriceUtil.hasPrice(root.item)

    description: {
        const price = PriceUtil.price(root.item);
        const currency = PriceUtil.currency(root.item);
        const homeCurrency = Country.fromAlpha2(Settings.homeCountryIsoCode).currencyCode;

        if (Settings.performCurrencyConversion && homeCurrency != currency) {
            const convertedPrice = UnitConversion.convertCurrency(price, currency, homeCurrency);
            if (!isNaN(convertedPrice) && convertedPrice != 0.0) {
                return i18nc("prices in local and home currency", "%1 (%2)",
                            Localizer.formatCurrency(price, currency),
                            Localizer.formatCurrency(convertedPrice, homeCurrency));
            }
        }

        return Localizer.formatCurrency(price, currency);
    }
}
