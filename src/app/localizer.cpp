/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "localizer.h"

#include <KItinerary/JsonLdDocument>
#include <KItinerary/Place>

#include <KContacts/Address>
#include <KFormat>
#include <KLocalizedString>

#include <QDateTime>
#include <QLocale>
#include <QTimeZone>

#ifdef Q_OS_ANDROID
#include <kandroidextras/jnisignature.h>
#include <kandroidextras/jnitypes.h>
#include <kandroidextras/locale.h>

#include <QtAndroid>
#include <QAndroidJniObject>

using namespace KAndroidExtras;
#endif

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

QString Localizer::formatAddress(const QVariant &obj) const
{
    if (!JsonLd::isA<PostalAddress>(obj)) {
        return {};
    }
    const auto a = obj.value<PostalAddress>();

    KContacts::Address address;
    address.setStreet(a.streetAddress());
    address.setPostalCode(a.postalCode());
    address.setLocality(a.addressLocality());
    address.setRegion(a.addressRegion());
    address.setCountry(KContacts::Address::ISOtoCountry(a.addressCountry()));

    return address.formattedAddress().replace(QLatin1String("\n\n"), QLatin1String("\n"));
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
