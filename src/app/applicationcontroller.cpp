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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "applicationcontroller.h"

#include <KItinerary/JsonLdDocument>
#include <KItinerary/Place>

#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniObject>
#else
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#endif

using namespace KItinerary;

ApplicationController::ApplicationController(QObject* parent)
    : QObject(parent)
{
}

ApplicationController::~ApplicationController() = default;

void ApplicationController::showOnMap(const QVariant &place)
{
    if (place.isNull()) {
        return;
    }

    const auto geo = JsonLdDocument::readProperty(place, "geo").value<GeoCoordinates>();
    const auto addr = JsonLdDocument::readProperty(place, "address").value<PostalAddress>();

#ifdef Q_OS_ANDROID
    QString intentUri;
    if (geo.isValid()) {
        intentUri = QLatin1String("geo:") + QString::number(geo.latitude())
            + QLatin1Char(',') + QString::number(geo.longitude());
    } else if (!addr.isEmpty()) {
        intentUri = QLatin1String("geo:0,0?q=") + addr.streetAddress() + QLatin1String(", ")
            + addr.postalCode() + QLatin1Char(' ')
            + addr.addressLocality() + QLatin1String(", ")
            + addr.addressCountry();
    } else {
        return;
    }

    const auto activity = QtAndroid::androidActivity();
    if (activity.isValid()) {
        activity.callMethod<void>("launchViewIntentFromUri", "(Ljava/lang/String;)V", QAndroidJniObject::fromString(intentUri).object());
    }

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

bool ApplicationController::canNavigateTo(const QVariant& place)
{
    if (place.isNull()) {
        return false;
    }

    if (JsonLdDocument::readProperty(place, "geo").value<GeoCoordinates>().isValid()) {
        return true;
    }

#ifdef Q_OS_ANDROID
    return !JsonLdDocument::readProperty(place, "address").value<PostalAddress>().isEmpty();
#else
    return false;
#endif
}

void ApplicationController::navigateTo(const QVariant& place)
{
    if (place.isNull()) {
        return;
    }

#ifdef Q_OS_ANDROID
    const auto geo = JsonLdDocument::readProperty(place, "geo").value<GeoCoordinates>();
    const auto addr = JsonLdDocument::readProperty(place, "address").value<PostalAddress>();

    QString intentUri;
    if (geo.isValid()) {
        intentUri = QLatin1String("google.navigation:q=") + QString::number(geo.latitude())
            + QLatin1Char(',') + QString::number(geo.longitude());
    } else if (!addr.isEmpty()) {
        intentUri = QLatin1String("google.navigation:q=") + addr.streetAddress() + QLatin1String(", ")
            + addr.postalCode() + QLatin1Char(' ')
            + addr.addressLocality() + QLatin1String(", ")
            + addr.addressCountry();
    } else {
        return;
    }

    const auto activity = QtAndroid::androidActivity();
    if (activity.isValid()) {
        activity.callMethod<void>("launchViewIntentFromUri", "(Ljava/lang/String;)V", QAndroidJniObject::fromString(intentUri).object());
    }

#else
    if (m_pendingNavigation) {
        return;
    }

    if (!m_positionSource) {
        m_positionSource = QGeoPositionInfoSource::createDefaultSource(this);
        if (!m_positionSource) {
            qWarning() << "no geo position info source available";
            return;
        }
    }

    if (m_positionSource->lastKnownPosition().isValid()) {
        navigateTo(m_positionSource->lastKnownPosition(), place);
    } else {
        m_pendingNavigation = connect(m_positionSource, &QGeoPositionInfoSource::positionUpdated,
                                      this, [this, place](const QGeoPositionInfo &pos) {
            navigateTo(pos, place);
        });
        m_positionSource->requestUpdate();
    }
#endif
}

#ifndef Q_OS_ANDROID
void ApplicationController::navigateTo(const QGeoPositionInfo &from, const QVariant &to)
{
    disconnect(m_pendingNavigation);
    if (!from.isValid()) {
        return;
    }

    const auto geo = JsonLdDocument::readProperty(to, "geo").value<GeoCoordinates>();
    if (geo.isValid()) {
        QUrl url;
        url.setScheme(QStringLiteral("https"));
        url.setHost(QStringLiteral("www.openstreetmap.org"));
        url.setPath(QStringLiteral("/directions"));
        const QString fragment = QLatin1String("route=") + QString::number(from.coordinate().latitude())
                                    + QLatin1Char(',') + QString::number(from.coordinate().longitude())
                                    + QLatin1Char(',') + QString::number(geo.latitude())
                                    + QLatin1Char(',') + QString::number(geo.longitude());
        url.setFragment(fragment);
        QDesktopServices::openUrl(url);
        return;
    }
}
#endif
