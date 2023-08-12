/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "unitconversion.h"
#include "logging.h"

#include <KUnitConversion/Value>

#include <QScopedValueRollback>

#include <cmath>

double UnitConversion::convertCurrency(double price, const QString &fromCurrency, const QString &toCurrency)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // HACK work around KUnitConversion's internal currency update event loop causing us to re-enter here again
    // which would then subsequently deadlock in KUnitConversion
    static bool s_entrancyGuard = false;
    if (s_entrancyGuard) {
        qCWarning(Log) << "Imminent KUnitConversion deadlock detected, skipping currency conversion!";
        return NAN;
    }
    QScopedValueRollback rollback(s_entrancyGuard, false);
    s_entrancyGuard = true;
#endif

    if (fromCurrency.isEmpty() || toCurrency.isEmpty() || fromCurrency == toCurrency) {
        return NAN;
    }

    const auto value = KUnitConversion::Value(price, fromCurrency).convertTo(toCurrency);
    return value.isValid() ? value.number() : NAN;
}
