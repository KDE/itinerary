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

#include "cache.h"

#include <KPublicTransport/Location>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

using namespace KPublicTransport;

static QString cachePath(const QString &backendId, const QString &contentType)
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) +
        QLatin1String("/org.kde.kpublictransport/") + backendId + QLatin1Char('/') +
        contentType + QLatin1Char('/');
}

void Cache::addLocationCacheEntry(const QString &backendId, const QString &cacheKey, const std::vector<Location> &data)
{
    const auto dir = cachePath(backendId, QStringLiteral("location"));
    QDir().mkpath(dir);
    QFile f(dir + cacheKey + QLatin1String(".json"));
    f.open(QFile::WriteOnly | QFile::Truncate);
    f.write(QJsonDocument(Location::toJson(data)).toJson());
}

void Cache::addNegativeLocationCacheEntry(const QString &backendId, const QString &cacheKey)
{
    const auto dir = cachePath(backendId, QStringLiteral("location"));
    QDir().mkpath(dir);
    QFile f(dir + cacheKey + QLatin1String(".json"));
    f.open(QFile::WriteOnly | QFile::Truncate);
    // empty file is used as indicator for a negative hit
}

CacheEntry<Location> Cache::lookupLocation(const QString &backendId, const QString &cacheKey)
{
    const auto dir = cachePath(backendId, QStringLiteral("location"));
    QFile f (dir + cacheKey + QLatin1String(".json"));
    if (!f.open(QFile::ReadOnly)) {
        return {{}, CacheHitType::Miss };
    }

    if (f.size() == 0) {
        return {{}, CacheHitType::Negative };
    }

    CacheEntry<Location> entry;
    entry.type = CacheHitType::Positive;
    entry.data = Location::fromJson(QJsonDocument::fromJson(f.readAll()).array());
    return entry;
}
