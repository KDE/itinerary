/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LOCALIZER_H
#define LOCALIZER_H

#include <KFormat>

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
    Q_INVOKABLE QString formatCountryWithContext(const QVariant &obj, const QVariant &otherObj, const QString &homeCountryIsoCode);
    Q_INVOKABLE QString formatTime(const QVariant &obj, const QString &propertyName) const;
    Q_INVOKABLE QString formatDate(const QVariant &obj, const QString &propertyName) const;
    Q_INVOKABLE QString formatDateTime(const QVariant &obj, const QString &propertyName) const;
    /** Auto-detect date or date/time, and convert to local time zone. */
    Q_INVOKABLE QString formatDateOrDateTimeLocal(const QVariant &obj, const QString &propertyName) const;
    Q_INVOKABLE [[nodiscard]] static QString formatTimeZoneOffset(qint64 seconds);
    Q_INVOKABLE [[nodiscard]] static QString formatDuration(int seconds);
    /** Format speed value. */
    Q_INVOKABLE [[nodiscard]] static QString formatSpeed(double km_per_hour, KFormat::DistanceFormatOptions formatOpts);
    /** Format a weight value. */
    Q_INVOKABLE static QString formatWeight(int gram);
    /** Format a temperature range, given in degree Celsius. */
    Q_INVOKABLE static QString formatTemperatureRange(double minTemperature, double maxTemperature, bool useFahrenheit);

    /** Format a currency value using the given currency to select the appropriate amount of decimals. */
    Q_INVOKABLE static QString formatCurrency(double value, const QString &isoCode);
};

#endif // LOCALIZER_H
