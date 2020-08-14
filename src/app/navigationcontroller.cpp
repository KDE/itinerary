/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "navigationcontroller.h"
#include "logging.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/Place>

#include <QCoreApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>

#ifdef Q_OS_ANDROID
#include <kandroidextras/activity.h>
#include <kandroidextras/intent.h>

#include <QtAndroid>
#include <QAndroidJniObject>
#else
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#endif

using namespace KItinerary;

#ifdef Q_OS_ANDROID
static bool startActivity(const QUrl &url)
{
    using namespace KAndroidExtras;

    Intent intent;
    intent.setAction(Intent::ACTION_VIEW);
    intent.setData(url);
    return Activity::startActivity(intent, 0);
}
#endif

void NavigationController::showOnMap(const QVariant &place)
{
    if (place.isNull()) {
        return;
    }

    const auto geo = LocationUtil::geo(place);
    const auto addr = LocationUtil::address(place);

#ifdef Q_OS_ANDROID
    QUrl url;
    url.setScheme(QStringLiteral("geo"));
    if (geo.isValid()) {
        url.setPath(QString::number(geo.latitude()) + QLatin1Char(',') + QString::number(geo.longitude()));
    } else if (!addr.isEmpty()) {
        url.setPath(QStringLiteral("0,0"));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("q"), addr.streetAddress() + QLatin1String(", ")
            + addr.postalCode() + QLatin1Char(' ')
            + addr.addressLocality() + QLatin1String(", ")
            + addr.addressCountry());
        url.setQuery(query);
    } else {
        return;
    }
    startActivity(url);

#else
    if (geo.isValid()) {
        // zoom out further from airports, they are larger and you usually want to go further away from them
        const auto zoom = place.userType() == qMetaTypeId<Airport>() ? 12 : 17;
        QUrl url;
        url.setScheme(QStringLiteral("https"));
        url.setHost(QStringLiteral("www.openstreetmap.org"));
        url.setPath(QStringLiteral("/"));
        const QString fragment = QLatin1String("map=") + QString::number(zoom)
                                    + QLatin1Char('/') + QString::number(geo.latitude())
                                    + QLatin1Char('/') + QString::number(geo.longitude());
        url.setFragment(fragment);
        QDesktopServices::openUrl(url);
        return;
    }

    if (!addr.isEmpty()) {
        QUrl url;
        url.setScheme(QStringLiteral("https"));
        url.setHost(QStringLiteral("www.openstreetmap.org"));
        url.setPath(QStringLiteral("/search"));
        const QString queryString = addr.streetAddress() + QLatin1String(", ")
                                    + addr.postalCode() + QLatin1Char(' ')
                                    + addr.addressLocality() + QLatin1String(", ")
                                    + addr.addressCountry();
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("query"), queryString);
        url.setQuery(query);
        QDesktopServices::openUrl(url);
    }
#endif
}

bool NavigationController::canNavigateTo(const QVariant& place)
{
    if (place.isNull()) {
        return false;
    }

    if (LocationUtil::geo(place).isValid()) {
        return true;
    }

#ifdef Q_OS_ANDROID
    return !LocationUtil::address(place).isEmpty();
#else
    return false;
#endif
}

void NavigationController::navigateTo(const QVariant& place)
{
    if (place.isNull()) {
        return;
    }

#ifdef Q_OS_ANDROID
    const auto geo = LocationUtil::geo(place);;
    const auto addr = LocationUtil::address(place);

    QUrl url;
    url.setScheme(QStringLiteral("google.navigation"));
    if (geo.isValid()) {
        url.setPath(QLatin1String("q=") + QString::number(geo.latitude())+ QLatin1Char(',') + QString::number(geo.longitude()));
    } else if (!addr.isEmpty()) {
        url.setPath(QLatin1String("q=") + addr.streetAddress() + QLatin1String(", ")
            + addr.postalCode() + QLatin1Char(' ')
            + addr.addressLocality() + QLatin1String(", ")
            + addr.addressCountry());
    } else {
        return;
    }
    startActivity(url);

#else
    if (m_pendingNavigation) {
        return;
    }

    if (!m_positionSource) {
        m_positionSource = QGeoPositionInfoSource::createDefaultSource(QCoreApplication::instance());
        if (!m_positionSource) {
            qWarning() << "no geo position info source available";
            return;
        }
    }

    if (m_positionSource->lastKnownPosition().isValid()) {
        navigateTo(m_positionSource->lastKnownPosition(), place);
    } else {
        m_pendingNavigation = QObject::connect(m_positionSource, &QGeoPositionInfoSource::positionUpdated, [this, place](const QGeoPositionInfo &pos) {
            navigateTo(pos, place);
        });
        m_positionSource->requestUpdate();
    }
#endif
}

#ifndef Q_OS_ANDROID
void NavigationController::navigateTo(const QGeoPositionInfo &from, const QVariant &to)
{
    qCDebug(Log) << from.coordinate() << from.isValid();
    QObject::disconnect(m_pendingNavigation);
    if (!from.isValid()) {
        return;
    }

    const auto geo = LocationUtil::geo(to);
    if (geo.isValid()) {
        QUrl url;
        url.setScheme(QStringLiteral("https"));
        url.setHost(QStringLiteral("www.openstreetmap.org"));
        url.setPath(QStringLiteral("/directions"));
        QUrlQuery query;
        query.addQueryItem(QLatin1String("route"),
            QString::number(from.coordinate().latitude()) + QLatin1Char(',') + QString::number(from.coordinate().longitude())
            + QLatin1Char(';') + QString::number(geo.latitude()) + QLatin1Char(',') + QString::number(geo.longitude()));
        url.setQuery(query);
        QDesktopServices::openUrl(url);
        return;
    }
}
#endif

void NavigationController::navigateTo(const QVariant& from, const QVariant& to)
{
    const auto fromGeo = LocationUtil::geo(from);
    const auto toGeo = LocationUtil::geo(to);
    if (!fromGeo.isValid() || !toGeo.isValid()) {
        navigateTo(to);
        return;
    }

#ifdef Q_OS_ANDROID
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("start_lat"), QString::number(fromGeo.latitude()));
    query.addQueryItem(QStringLiteral("start_lon"), QString::number(fromGeo.longitude()));
    query.addQueryItem(QStringLiteral("dest_lat"), QString::number(toGeo.latitude()));
    query.addQueryItem(QStringLiteral("dest_lon"), QString::number(toGeo.longitude()));
    query.addQueryItem(QStringLiteral("start_name"), LocationUtil::name(from));
    query.addQueryItem(QStringLiteral("dest_name"), LocationUtil::name(to));
    QUrl url;
    url.setScheme(QStringLiteral("osmand.api"));
    url.setHost(QStringLiteral("navigate"));
    url.setQuery(query);
    if (!startActivity(url)) {
        navigateTo(to);
    }

#else
    QUrl url;
    url.setScheme(QStringLiteral("https"));
    url.setHost(QStringLiteral("www.openstreetmap.org"));
    url.setPath(QStringLiteral("/directions"));
    QUrlQuery query;
    query.addQueryItem(QLatin1String("route"),
        QString::number(fromGeo.latitude()) + QLatin1Char(',') + QString::number(fromGeo.longitude())
        + QLatin1Char(';') + QString::number(toGeo.latitude()) + QLatin1Char(',') + QString::number(toGeo.longitude()));
    url.setQuery(query);
    QDesktopServices::openUrl(url);
#endif
}
