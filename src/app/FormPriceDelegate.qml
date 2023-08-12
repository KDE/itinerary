// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.i18n.localeData 1.0
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

MobileForm.FormTextDelegate {
    id: root

    /** The element of which the price properties should be shown. */
    property var item

    text: i18n("Price")
    visible: PriceUtil.hasPrice(root.item)

    description: {
        const currency = PriceUtil.currency(root.item);
        const homeCurrency = Country.fromAlpha2(Settings.homeCountryIsoCode).currencyCode;
        if (Settings.performCurrencyConversion && homeCurrency != currency) {
            // TODO show price in both currencies
        }

        Localizer.formatCurrency(PriceUtil.price(root.item), PriceUtil.currency(root.item))
    }
}
