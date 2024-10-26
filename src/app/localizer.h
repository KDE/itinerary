/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LOCALIZER_H
#define LOCALIZER_H

#include <QObject>

class QVariant;

/** Date/time localization utilities.
 *  Works around JS losing timezone information, ie. we need
 *  to do this without passing the date/time values through JS.
 */
class Localizer
{
    Q_GADGET
public:
    Q_INVOKABLE QString formatAddress(const QVariant &obj) const;
    Q_INVOKABLE QString formatAddressWithContext(const QVariant &obj, const QVariant &otherObj, const QString &homeCountryIsoCode);
    Q_INVOKABLE QString formatTime(const QVariant &obj, const QString &propertyName) const;
    Q_INVOKABLE QString formatDate(const QVariant &obj, const QString &propertyName) const;
    Q_INVOKABLE QString formatDateTime(const QVariant &obj, const QString &propertyName) const;
    /** Auto-detect date or date/time, and convert to local time zone. */
    Q_INVOKABLE QString formatDateOrDateTimeLocal(const QVariant &obj, const QString &propertyName) const;
    Q_INVOKABLE QString formatDuration(int seconds) const;
    /** Format a distance value. */
    Q_INVOKABLE static QString formatDistance(int meter);
    /** Format speed value. */
    Q_INVOKABLE static QString formatSpeed(int km_per_hour);
    /** Format a weight value. */
    Q_INVOKABLE static QString formatWeight(int gram);
    /** Format a temperature, given in degree Celsius. */
    Q_INVOKABLE static QString formatTemperature(double temperature);

    /** Format a currency value using the given currency to select the appropriate amount of decimals. */
    Q_INVOKABLE static QString formatCurrency(double value, const QString &isoCode);
};

#endif // LOCALIZER_H
