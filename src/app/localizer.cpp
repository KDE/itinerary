/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "localizer.h"

#include <KItinerary/JsonLdDocument>
#include <KItinerary/Place>

#include <KContacts/Address>

#include <KCountry>
#include <KFormat>
#include <KLocalizedString>

#include <QDateTime>
#include <QLocale>
#include <QMetaProperty>
#include <QTimeZone>

#ifdef Q_OS_ANDROID
#include <kandroidextras/javatypes.h>
#include <kandroidextras/jnisignature.h>
#include <kandroidextras/javalocale.h>

using namespace KAndroidExtras;
#endif

#include <cstring>

using namespace KItinerary;

static QString readFromGadget(const QMetaObject *mo, const QVariant &gadget, const char *propName)
{
    const auto propIdx = mo->indexOfProperty(propName);
    if (propIdx < 0) {
        return {};
    }
    const auto prop = mo->property(propIdx);
    if (!prop.isValid()) {
        return {};
    }
    return prop.readOnGadget(gadget.constData()).toString();
}

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
    } else if (std::strcmp(obj.typeName(), "KOSMIndoorMap::OSMAddress") == 0) {
        const auto mo = QMetaType(obj.userType()).metaObject();
        address.setStreet(readFromGadget(mo, obj, "street") + QLatin1Char(' ') + readFromGadget(mo, obj, "houseNumber"));
        address.setPostalCode(readFromGadget(mo, obj, "postalCode"));
        address.setLocality(readFromGadget(mo, obj, "city"));
        address.setRegion(readFromGadget(mo, obj, "state"));
        address.setCountry(readFromGadget(mo, obj, "country"));
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
    return address.street().isEmpty() && address.locality().isEmpty()
            && address.postalCode().isEmpty() && address.region().isEmpty()
            && !address.country().isEmpty();
}

QString Localizer::formatAddressWithContext(const QVariant &obj, const QVariant &otherObj, const QString &homeCountryIsoCode)
{
    const KContacts::Address address = variantToKContactsAddress(obj);

    if (address.isEmpty()) {
        return {};
    }

    const bool includeCountry = [&] {
        if (homeCountryIsoCode.isEmpty()) {
            return true;
        }

        if (!addressEmptyExceptForCountry(address)) {
            return true;
        }

        if (address.country() != homeCountryIsoCode) {
            return true;
        }

        const KContacts::Address otherAddress = variantToKContactsAddress(otherObj);
        if (!otherAddress.country().isEmpty() && address.country() != otherAddress.country()) {
            return true;
        }

        return false;
    }();

    if (includeCountry) {
        return address.formatted(KContacts::AddressFormatStyle::MultiLineInternational);
    } else {
        return address.formatted(KContacts::AddressFormatStyle::MultiLineDomestic);
    }
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
    auto abbr = QAndroidJniObject::callStaticObjectMethod("org/kde/itinerary/QTimeZone", "abbreviation",
                    Jni::signature<java::lang::String(java::lang::String, jlong, java::util::Locale, bool)>(),
                    QAndroidJniObject::fromString(QString::fromUtf8(tz.id())).object(), dt.toMSecsSinceEpoch(),
                    KAndroidExtras::Locale::current().object(), tz.isDaylightTime(dt)).toString();

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

QString Localizer::formatDateTime(const QVariant& obj, const QString& propertyName) const
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

QString Localizer::formatDateOrDateTimeLocal(const QVariant& obj, const QString& propertyName) const
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

QString Localizer::formatDuration(int seconds) const
{
    if (seconds < 0) {
        return QLocale().negativeSign() + KFormat().formatDuration((-seconds * 1000), KFormat::HideSeconds);
    }
    return KFormat().formatDuration(seconds * 1000, KFormat::HideSeconds);
}

QString Localizer::formatDistance(int meter)
{
    if (meter < 1000) {
        return i18nc("distance in meter", "%1 m", meter);
    }
    if (meter < 10000) {
        return i18nc("distance in kilometer", "%1 km", ((int)meter/100)/10.0);
    }
    return i18nc("distance in kilometer", "%1 km", (int)qRound(meter/1000.0));
}

QString Localizer::formatSpeed(int km_per_hour)
{
    // TODO locale-specific unit conversion
    return i18nc("speed in kilometers per hour", "%1 km/h", km_per_hour);
}

QString Localizer::formatWeight(int gram)
{
    if (gram < 1000) {
        return i18nc("weight in gram", "%1 g", gram);
    }
    if (gram < 10000) {
        return i18nc("weight in kilogram", "%1 kg", ((int)gram/100)/10.0);
    }
    return i18nc("weight in kilogram", "%1 kg", (int)qRound(gram/1000.0));

}

QString Localizer::formatTemperature(double temperature)
{
    return i18nc("temperature", "%1°C", (int)qRound(temperature));
}
