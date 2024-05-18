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

using namespace Qt::Literals::StringLiterals;

[[nodiscard]] static QString basePath(LiveData::Type type)
{
    QString typeStr;
    switch (type) {
        case LiveData::Departure:
            typeStr = u"departure"_s;
            break;
        case LiveData::Arrival:
            typeStr = u"arrival"_s;
            break;
        case LiveData::Journey:
            typeStr = u"journey"_s;
            break;
        default:
            assert(false);
    }
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/publictransport/"_L1 + typeStr + '/'_L1;
}

static QJsonObject loadOne(const QString &resId, LiveData::Type type, QDateTime &timestamp)
{
    const auto path = basePath(type);

    QFile f(path + resId + ".json"_L1);
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

    const QString fileName = path + resId + ".json"_L1;

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

QJsonObject LiveData::toJson(const LiveData &ld)
{
    QJsonObject obj;
    obj.insert("departure"_L1, KPublicTransport::Stopover::toJson(ld.departure));
    obj.insert("departureTimestamp"_L1, ld.departureTimestamp.toString(Qt::ISODate));
    obj.insert("arrival"_L1, KPublicTransport::Stopover::toJson(ld.arrival));
    obj.insert("arrivalTimestamp"_L1, ld.arrivalTimestamp.toString(Qt::ISODate));
    obj.insert("journey"_L1, KPublicTransport::JourneySection::toJson(ld.journey));
    obj.insert("journeyTimestamp"_L1, ld.journeyTimestamp.toString(Qt::ISODate));
    return obj;
}

LiveData LiveData::fromJson(const QJsonObject &obj)
{
    LiveData ld;
    ld.departure = KPublicTransport::Stopover::fromJson(obj.value("departure"_L1).toObject());
    ld.departureTimestamp = QDateTime::fromString(obj.value("departureTimestamp"_L1).toString(), Qt::ISODate);
    ld.arrival = KPublicTransport::Stopover::fromJson(obj.value("arrival"_L1).toObject());
    ld.arrivalTimestamp = QDateTime::fromString(obj.value("arrivalTimestamp"_L1).toString(), Qt::ISODate);
    ld.journey = KPublicTransport::JourneySection::fromJson(obj.value("journey"_L1).toObject());
    ld.journeyTimestamp = QDateTime::fromString(obj.value("journeyTimestamp"_L1).toString(), Qt::ISODate);
    return ld;
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
