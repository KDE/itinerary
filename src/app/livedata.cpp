/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

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

#include "livedata.h"
#include "logging.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

static QString basePath(LiveData::Type type)
{
    QString typeStr;
    switch (type) {
        case LiveData::Departure:
            typeStr = QStringLiteral("departure");
            break;
        case LiveData::Arrival:
            typeStr = QStringLiteral("arrival");
            break;
        case LiveData::Journey:
            typeStr = QStringLiteral("journey");
            break;
        default:
            assert(false);
    }
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/publictransport/") + typeStr + QLatin1Char('/');
}

static QJsonObject loadOne(const QString &resId, LiveData::Type type, QDateTime &timestamp)
{
    const auto path = basePath(type);

    QFile f(path + resId + QLatin1String(".json"));
    if (!f.open(QFile::ReadOnly)) {
        timestamp = {};
        return {};
    }

    timestamp = f.fileTime(QFile::FileModificationTime);
    return QJsonDocument::fromJson(f.readAll()).object();
}

LiveData LiveData::load(const QString &resId)
{
    LiveData ld;
    auto obj = loadOne(resId, Departure, ld.departureTimestamp);
    ld.departure = KPublicTransport::Stopover::fromJson(obj);
    obj = loadOne(resId, Arrival, ld.arrivalTimestamp);
    ld.arrival = KPublicTransport::Stopover::fromJson(obj);
    obj = loadOne(resId, Journey, ld.journeyTimestamp);
    ld.journey = KPublicTransport::JourneySection::fromJson(obj);
    return ld;
}

static void storeOne(const QString &resId, LiveData::Type type, const QJsonObject &obj, const QDateTime &dt)
{
    const auto path = basePath(type);
    QDir().mkpath(path);

    const QString fileName = path + resId + QLatin1String(".json");

    if (obj.isEmpty()) {
        QFile::remove(fileName);
    } else {
        QFile file(fileName);
        if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
            qCWarning(Log) << "Failed to open public transport cache file:" << file.fileName() << file.errorString();
            return;
        }
        file.write(QJsonDocument(obj).toJson());
        file.close();

        // mtime changes need to be done without content changes to take effect
        file.open(QFile::WriteOnly | QFile::Append);
        file.setFileTime(dt, QFile::FileModificationTime);
        file.close();
    }
}

void LiveData::store(const QString &resId, int types) const
{
    if (types & Departure) {
        storeOne(resId, Departure, KPublicTransport::Stopover::toJson(departure), departureTimestamp);
    }
    if (types & Arrival) {
        storeOne(resId, Arrival, KPublicTransport::Stopover::toJson(arrival), arrivalTimestamp);
    }
    if (types & Journey) {
        storeOne(resId, Journey, KPublicTransport::JourneySection::toJson(journey), journeyTimestamp);
    }
}

static void listOne(LiveData::Type type, std::vector<QString> &ids)
{
    QDir dir(basePath(type));
    QDirIterator it(basePath(type), QDir::Files);
    while (it.hasNext()) {
        it.next();
        const auto id = it.fileInfo().baseName();
        const auto idIt = std::lower_bound(ids.begin(), ids.end(), id);
        if (idIt != ids.end() && (*idIt) == id) {
            continue;
        }
        ids.insert(idIt, id);
    }
}

std::vector<QString> LiveData::listAll()
{
    std::vector<QString> ids;
    for (auto type : { Departure, Arrival, Journey }) {
        listOne(type, ids);
    }
    return ids;
}

void LiveData::clearStorage()
{
    for (auto type : { Departure, Arrival, Journey }) {
        const auto path = basePath(type);
        if (path.isEmpty()) {
            continue; // just to not accidentally kill everything...
        }
        QDir(path).removeRecursively();
    }
}
