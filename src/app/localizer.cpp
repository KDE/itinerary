/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "localizer.h"

#include <KItinerary/JsonLdDocument>
#include <KItinerary/Place>
#include <KItinerary/PriceUtil>

#include <KPublicTransport/Location>

#include <KContacts/Address>

#include <KUnitConversion/Converter>

#include <KCountry>
#include <KFormat>
#include <KLocalizedString>

#include <QDateTime>
#include <QLocale>
#include <QMetaProperty>
#include <QTimeZone>

#ifdef Q_OS_ANDROID
#include "kandroidextras/javalocale.h"
#include "kandroidextras/javatypes.h"
#include "kandroidextras/jnisignature.h"

using namespace KAndroidExtras;
#endif

#include <cmath>
#include <cstring>

using namespace KItinerary;
using namespace Qt::StringLiterals;

static KContacts::Address variantToKContactsAddress(const QVariant &obj)
{
    KContacts::Address address;
    if (JsonLd::isA<PostalAddress>(obj)) {
        const auto a = obj.value<PostalAddress>();
        address.setStreet(a.streetAddress());
        address.setPostalCode(a.postalCode());
        address.setLocality(a.addressLocality());
        address.setRegion(a.addressRegion());
        address.setCountry(a.addressCountry());
    }
    if (obj.typeId() == qMetaTypeId<KPublicTransport::Location>()) {
        const auto a = obj.value<KPublicTransport::Location>();
        address.setStreet(a.streetAddress());
        address.setPostalCode(a.postalCode());
        address.setLocality(a.locality());
        address.setRegion(a.region());
        address.setCountry(a.country());
    }
    return address;
}

QString Localizer::formatAddress(const QVariant &obj) const
{
    KContacts::Address address = variantToKContactsAddress(obj);
    if (address.isEmpty()) {
        return {};
    }
    return address.formatted(KContacts::AddressFormatStyle::MultiLineInternational);
}

static bool addressEmptyExceptForCountry(const KContacts::Address &address)
{
    return address.street().isEmpty() && address.locality().isEmpty() && address.postalCode().isEmpty() && address.region().isEmpty()
        && !address.country().isEmpty();
}

static bool includeCountry(const KContacts::Address &address, const QVariant &otherObj, const QString &homeCountryIsoCode)
{
    if (homeCountryIsoCode.isEmpty()) {
        return true;
    }

    if (address.country() != homeCountryIsoCode) {
        return true;
    }

    const KContacts::Address otherAddress = variantToKContactsAddress(otherObj);
    return !otherAddress.country().isEmpty() && address.country() != otherAddress.country();
};

QString Localizer::formatCountryWithContext(const QVariant &obj, const QVariant &otherObj, const QString &homeCountryIsoCode)
{
    const KContacts::Address address = variantToKContactsAddress(obj);

    if (address.isEmpty()) {
        return {};
    }

    if (includeCountry(address, otherObj, homeCountryIsoCode)) {
        return KCountry::fromAlpha2(address.country()).name();
    }
    return {};
}

QString Localizer::formatAddressWithContext(const QVariant &obj, const QVariant &otherObj, const QString &homeCountryIsoCode)
{
    const KContacts::Address address = variantToKContactsAddress(obj);

    if (address.isEmpty()) {
        return {};
    }

    if (!addressEmptyExceptForCountry(address) || includeCountry(address, otherObj, homeCountryIsoCode)) {
        return address.formatted(KContacts::AddressFormatStyle::MultiLineInternational);
    }
    return address.formatted(KContacts::AddressFormatStyle::MultiLineDomestic);
}

static bool needsTimeZone(const QDateTime &dt)
{
    if (dt.timeSpec() == Qt::TimeZone && dt.timeZone().abbreviation(dt) != QTimeZone::systemTimeZone().abbreviation(dt)) {
        return true;
    } else if (dt.timeSpec() == Qt::OffsetFromUTC && dt.timeZone().offsetFromUtc(dt) != dt.offsetFromUtc()) {
        return true;
    } else if (dt.timeSpec() == Qt::UTC && QTimeZone::systemTimeZone() != QTimeZone::utc()) {
        return true;
    }
    return false;
}

static QString tzAbbreviation(const QDateTime &dt)
{
    const auto tz = dt.timeZone();

#ifdef Q_OS_ANDROID
    // the QTimeZone backend implementation on Android isn't as complete as the desktop ones, so we need to do this ourselves here
    // eventually, this should be upstreamed to Qt
    auto abbr = QJniObject::callStaticObjectMethod("org/kde/itinerary/QTimeZone",
                                                   "abbreviation",
                                                   Jni::signature<java::lang::String(java::lang::String, jlong, java::util::Locale, bool)>(),
                                                   QJniObject::fromString(QString::fromUtf8(tz.id())).object(),
                                                   dt.toMSecsSinceEpoch(),
                                                   KAndroidExtras::Locale::current().object(),
                                                   tz.isDaylightTime(dt))
                    .toString();

    if (!abbr.isEmpty()) {
        return abbr;
    }
#endif

    return tz.abbreviation(dt);
}

QString Localizer::formatTime(const QVariant &obj, const QString &propertyName) const
{
    const auto dt = JsonLdDocument::readProperty(obj, propertyName.toUtf8().constData()).toDateTime();
    if (!dt.isValid()) {
        return {};
    }

    QString output;
    if (QLocale().timeFormat(QLocale::ShortFormat).contains(QStringLiteral("ss"))) {
        output = QLocale().toString(dt.time(), QStringLiteral("hh:mm"));
    } else {
        output = QLocale().toString(dt.time(), QLocale::ShortFormat);
    }
    if (needsTimeZone(dt)) {
        output += QLatin1Char(' ') + tzAbbreviation(dt);
    }
    return output;
}

QString Localizer::formatDate(const QVariant &obj, const QString &propertyName) const
{
    const auto dt = JsonLdDocument::readProperty(obj, propertyName.toUtf8().constData()).toDate();
    if (!dt.isValid()) {
        return {};
    }

    if (dt.year() <= 1900) { // no year specified
        return dt.toString(i18nc("day-only date format", "dd MMMM"));
    }
    return QLocale().toString(dt, QLocale::ShortFormat);
}

QString Localizer::formatDateTime(const QVariant &obj, const QString &propertyName) const
{
    const auto dt = JsonLdDocument::readProperty(obj, propertyName.toUtf8().constData()).toDateTime();
    if (!dt.isValid()) {
        return {};
    }

    auto s = QLocale().toString(dt, QLocale::ShortFormat);
    if (needsTimeZone(dt)) {
        s += QLatin1Char(' ') + tzAbbreviation(dt);
    }
    return s;
}

QString Localizer::formatDateOrDateTimeLocal(const QVariant &obj, const QString &propertyName) const
{
    const auto dt = JsonLdDocument::readProperty(obj, propertyName.toUtf8().constData()).toDateTime();
    if (!dt.isValid()) {
        return {};
    }

    // detect likely date-only values
    if (dt.timeSpec() == Qt::LocalTime && (dt.time() == QTime{0, 0, 0} || dt.time() == QTime{23, 59, 59})) {
        return QLocale().toString(dt.date(), QLocale::ShortFormat);
    }

    return QLocale().toString(dt.toLocalTime(), QLocale::ShortFormat);
}

QString Localizer::formatTimeZoneOffset(qint64 seconds)
{
    if (seconds < 0) {
        return QLocale().negativeSign() + KFormat().formatDuration((-seconds * 1000), KFormat::HideSeconds);
    }
    return QLocale().positiveSign() + KFormat().formatDuration((seconds * 1000), KFormat::HideSeconds);
}

QString Localizer::formatDuration(int seconds)
{
    const auto opts = static_cast<KFormat::DurationFormatOptions>(KFormat::AbbreviatedDuration | KFormat::HideSeconds); // ### workaround for KF < 6.12
    return KFormat().formatDuration((quint64)seconds * 1000, opts);
}

QString Localizer::formatSpeed(double km_per_hour, KFormat::DistanceFormatOptions formatOpts)
{
    auto targetUnit = KUnitConversion::UnitId::KilometerPerHour;
    if (const auto ms = QLocale().measurementSystem(); (ms == QLocale::ImperialUKSystem || ms == QLocale::ImperialUSSystem) && (formatOpts & KFormat::MetricDistanceUnits) == 0) {
        targetUnit = KUnitConversion::UnitId::MilePerHour;
    }

    return KUnitConversion::Converter().convert(KUnitConversion::Value(km_per_hour, KUnitConversion::UnitId::KilometerPerHour), targetUnit).round(0).toSymbolString();
}

QString Localizer::formatWeight(int gram)
{
    if (gram < 1000) {
        return i18nc("weight in gram", "%1 g", gram);
    }
    if (gram < 10000) {
        return i18nc("weight in kilogram", "%1 kg", ((int)gram / 100) / 10.0);
    }
    return i18nc("weight in kilogram", "%1 kg", (int)qRound(gram / 1000.0));
}

QString Localizer::formatTemperatureRange(double minTemperature, double maxTemperature, bool useFahrenheit)
{
    using KUnitConversion::UnitId;

    const auto targetUnit = useFahrenheit ? UnitId::Fahrenheit : UnitId::Celsius;

    const auto minVal = KUnitConversion::Converter().convert(KUnitConversion::Value(minTemperature, UnitId::Celsius), targetUnit).round(0);
    const auto maxVal = KUnitConversion::Converter().convert(KUnitConversion::Value(maxTemperature, UnitId::Celsius), targetUnit).round(0);

    if (minVal.number() == maxVal.number()) {
        return minVal.toSymbolString();
    }
    return i18nc("temperature range", "%1 / %2", minVal.toSymbolString(), maxVal.toSymbolString());
}

QString Localizer::formatCurrency(double value, const QString &isoCode)
{
    const auto decimalCount = PriceUtil::decimalCount(isoCode);

    // special case for displaying conversion rates (which can be very small)
    // and thus need a higher precision than regular values
    double i = 0.0;
    double f = std::modf(value * std::pow(10, decimalCount), &i);
    if (i == 0.0 && f > 0.0) {
        return QLocale().toCurrencyString(value, isoCode);
    }

    return QLocale().toCurrencyString(value, isoCode, decimalCount);
}

#include "moc_localizer.cpp"
