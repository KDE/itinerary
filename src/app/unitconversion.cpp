/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "unitconversion.h"
#include "logging.h"

#include <KUnitConversion/Converter>
#include <KUnitConversion/Unit>
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

void UnitConversion::syncCurrencyConversionTable()
{
    const KUnitConversion::Converter converter;
    auto currencyCategory = converter.category(KUnitConversion::CurrencyCategory);
    auto *job = currencyCategory.syncConversionTable();
    if (job) {
        qCDebug(Log) << "Updating currency conversion table";
        QObject::connect(job, &KUnitConversion::UpdateJob::finished, job, [] {
            qCDebug(Log) << "Updated currency conversion table";
        });
    } else {
        qCDebug(Log) << "Currency conversion table already up to date";
    }
}

#include "moc_unitconversion.cpp"
