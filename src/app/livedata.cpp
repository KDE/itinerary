/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "livedata.h"

#include "jsonio.h"
#include "logging.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
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
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1StringView("/publictransport/") + typeStr + QLatin1Char('/');
}

static QJsonObject loadOne(const QString &resId, LiveData::Type type, QDateTime &timestamp)
{
    const auto path = basePath(type);

    QFile f(path + resId + QLatin1StringView(".json"));
    if (!f.open(QFile::ReadOnly)) {
        timestamp = {};
        return {};
    }

    timestamp = f.fileTime(QFile::FileModificationTime);
    return JsonIO::read(f.readAll()).toObject();
}

KPublicTransport::Stopover LiveData::stopover(LiveData::Type type) const
{
    assert(type == Arrival || type == Departure);
    return type == Arrival ? arrival : departure;
}

void LiveData::setStopover(LiveData::Type type, const KPublicTransport::Stopover &stop)
{
    assert(type == Arrival || type == Departure);
    type == Arrival ? arrival = stop : departure = stop;
}

void LiveData::setTimestamp(LiveData::Type type, const QDateTime &dt)
{
    switch (type) {
        case LiveData::Departure: departureTimestamp = dt; break;
        case LiveData::Arrival: arrivalTimestamp = dt; break;
        case LiveData::Journey: journeyTimestamp = dt; break;
        default: assert(false);
    }
}

bool LiveData::isEmpty() const
{
    return departure.stopPoint().isEmpty() && arrival.stopPoint().isEmpty() && journey.from().isEmpty() && journey.to().isEmpty();
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

    const QString fileName = path + resId + QLatin1StringView(".json");

    if (obj.isEmpty()) {
        QFile::remove(fileName);
    } else {
        QFile file(fileName);
        if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
            qCWarning(Log) << "Failed to open public transport cache file:" << file.fileName() << file.errorString();
            return;
        }
        file.write(JsonIO::write(obj));
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

void LiveData::remove(const QString& resId)
{
    for (auto type : { Departure, Arrival, Journey }) {
        storeOne(resId, type, {}, {});
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
