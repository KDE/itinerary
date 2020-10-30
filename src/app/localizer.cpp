/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "localizer.h"

#include <KItinerary/JsonLdDocument>
#include <KItinerary/Place>

#include <KContacts/Address>
#include <KFormat>
#include <KLocalizedString>

#include <QDateTime>
#include <QLocale>
#include <QMetaProperty>
#include <QTimeZone>

#ifdef Q_OS_ANDROID
#include <kandroidextras/jnisignature.h>
#include <kandroidextras/jnitypes.h>
#include <kandroidextras/javalocale.h>

#include <QtAndroid>
#include <QAndroidJniObject>

using namespace KAndroidExtras;
#endif

#include <cstring>

using namespace KItinerary;

QString Localizer::countryName(const QString& isoCode) const
{
    return KContacts::Address::ISOtoCountry(isoCode);
}

QString Localizer::countryFlag(const QString& isoCode) const
{
    if (isoCode.size() != 2) {
        return {};
    }

    QString flag;
    char flagA[] = "\xF0\x9F\x87\xA6";
    flagA[3] = 0xA6 + (isoCode[0].toLatin1() - 'A');
    flag += QString::fromUtf8(flagA);
    flagA[3] = 0xA6 + (isoCode[1].toLatin1() - 'A');
    flag += QString::fromUtf8(flagA);
    return flag;
}

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

QString Localizer::formatAddress(const QVariant &obj) const
{
    KContacts::Address address;
    if (JsonLd::isA<PostalAddress>(obj)) {
        const auto a = obj.value<PostalAddress>();
        address.setStreet(a.streetAddress());
        address.setPostalCode(a.postalCode());
        address.setLocality(a.addressLocality());
        address.setRegion(a.addressRegion());
        address.setCountry(KContacts::Address::ISOtoCountry(a.addressCountry()));
    } else if (std::strcmp(obj.typeName(), "KOSMIndoorMap::OSMAddress") == 0) {
        const auto mo = QMetaType::metaObjectForType(obj.userType());
        address.setStreet(readFromGadget(mo, obj, "street") + QLatin1Char(' ') + readFromGadget(mo, obj, "houseNumber"));
        address.setPostalCode(readFromGadget(mo, obj, "postalCode"));
        address.setLocality(readFromGadget(mo, obj, "city"));
        address.setRegion(readFromGadget(mo, obj, "state"));
        address.setCountry(KContacts::Address::ISOtoCountry(readFromGadget(mo, obj, "country")));
    } else {
        return {};
    }

    return address.formattedAddress().replace(QLatin1String("\n\n"), QLatin1String("\n")).trimmed();
}

static bool needsTimeZone(const QDateTime &dt)
{
    if (dt.timeSpec() == Qt::TimeZone && dt.timeZone().abbreviation(dt) != QTimeZone::systemTimeZone().abbreviation(dt)) {
        return true;
    } else if (dt.timeSpec() == Qt::OffsetFromUTC && dt.timeZone().offsetFromUtc(dt) != dt.offsetFromUtc()) {
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
                    Jni::signature<java::lang::String(java::lang::String, long, java::util::Locale, bool)>(),
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

    auto s = QLocale().toString(dt.time(), QLocale::ShortFormat);
    if (needsTimeZone(dt)) {
        s += QLatin1Char(' ') + tzAbbreviation(dt);
    }
    return s;
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

QString Localizer::formatDuration(int seconds) const
{
    if (seconds < 0) {
        return QLocale().negativeSign() + KFormat().formatDuration((-seconds * 1000), KFormat::HideSeconds);
    }
    return KFormat().formatDuration(seconds * 1000, KFormat::HideSeconds);
}

QString Localizer::formatDistance(int meter) const
{
    if (meter < 1000) {
        return i18n("%1m", meter);
    }
    if (meter < 10000) {
        return i18n("%1km", ((int)meter/100)/10.0);
    }
    return i18n("%1km", (int)qRound(meter/1000.0));
}
