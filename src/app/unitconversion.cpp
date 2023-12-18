/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "unitconversion.h"
#include "logging.h"

#include <KUnitConversion/Value>

#include <cmath>

double UnitConversion::convertCurrency(double price, const QString &fromCurrency, const QString &toCurrency)
{
    if (fromCurrency.isEmpty() || toCurrency.isEmpty() || fromCurrency == toCurrency) {
        return NAN;
    }

    const auto value = KUnitConversion::Value(price, fromCurrency).convertTo(toCurrency);
    return value.isValid() ? value.number() : NAN;
}

#include "moc_unitconversion.cpp"
