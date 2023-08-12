/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef UNITCONVERSION_H
#define UNITCONVERSION_H

#include <qobjectdefs.h>

/** QML access to KUnitConversion. */
class UnitConversion
{
    Q_GADGET
public:
    Q_INVOKABLE static double convertCurrency(double value, const QString &fromCurrency, const QString &toCurrency);
};

#endif // PERMISSIONMANAGER_H
