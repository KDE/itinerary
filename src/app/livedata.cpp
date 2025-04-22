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

bool LiveData::isEmpty() const
{
    return trip.from().isEmpty() && trip.to().isEmpty();
}

KPublicTransport::JourneySection LiveData::journey() const
{
    return (departureIndex < 0 || arrivalIndex < 0) ? trip : trip.subsection(departureIndex, arrivalIndex);
}

KPublicTransport::Stopover LiveData::departure() const
{
    return departureIndex < 0 ? trip.departure() : trip.stopover(departureIndex);
}

KPublicTransport::Stopover LiveData::arrival() const
{
    return arrivalIndex < 0 ? trip.arrival() : trip.stopover(arrivalIndex);
}

LiveData LiveData::load(const QString &resId)
{
    LiveData ld;

    QFile f(basePath() + resId + ".json"_L1);
    if (!f.open(QFile::ReadOnly)) {
        return ld;
    }

    ld.journeyTimestamp = f.fileTime(QFile::FileModificationTime);
    ld.trip = KPublicTransport::JourneySection::fromJson(JsonIO::read(f.readAll()).toObject());

    QFile metaFile(basePath() + resId + ".meta"_L1);
    if (metaFile.open(QFile::ReadOnly)) {
        const auto metaObj = JsonIO::read(metaFile.readAll()).toObject();
        ld.departureIndex = metaObj.value("departureIndex"_L1).toInteger(-1);
        ld.arrivalIndex = metaObj.value("arrivalIndex"_L1).toInteger(-1);
    }

    ld.recoverIndexes();
    return ld;
}

void LiveData::store(const QString &resId) const
{
    const auto path = basePath();
    QDir().mkpath(path);

    const QString fileName = path + resId + ".json"_L1;
    const QString metaFileName = path + resId + ".meta"_L1;

    const auto obj = KPublicTransport::JourneySection::toJson(trip);
    if (obj.isEmpty()) {
        QFile::remove(fileName);
        QFile::remove(metaFileName);
    } else {
        QFile file(fileName);
        if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
            qCWarning(Log) << "Failed to open trip data file:" << file.fileName() << file.errorString();
            return;
        }
        file.write(JsonIO::write(obj));
        file.close();

        // mtime changes need to be done without content changes to take effect
        file.open(QFile::WriteOnly | QFile::Append);
        file.setFileTime(journeyTimestamp, QFile::FileModificationTime);
        file.close();

        QFile metaFile(metaFileName);
        if (!metaFile.open(QFile::WriteOnly | QFile::Truncate)) {
            qCWarning(Log) << "Failed to open public trip metadata file:" << metaFile.fileName() << metaFile.errorString();
            return;
        }
        QJsonObject metaObj{
            {"departureIndex"_L1, departureIndex},
            {"arrivalIndex"_L1, arrivalIndex}
        };
        metaFile.write(JsonIO::write(metaObj));
    }
}

void LiveData::remove(const QString &resId)
{
    QFile::remove(basePath() + resId + ".json"_L1);
    QFile::remove(basePath() + resId + ".meta"_L1);
}

std::vector<QString> LiveData::listAll()
{
    std::vector<QString> ids;
    for (QDirIterator it(basePath(), {"*.json"_L1}, QDir::Files); it.hasNext();) {
        it.next();
        ids.push_back(it.fileInfo().baseName());
    }
    return ids;
}

QJsonObject LiveData::toJson(const LiveData &ld)
{
    QJsonObject obj;
    obj.insert("journey"_L1, KPublicTransport::JourneySection::toJson(ld.trip));
    obj.insert("journeyTimestamp"_L1, ld.journeyTimestamp.toString(Qt::ISODate));
    obj.insert("departureIndex"_L1, ld.departureIndex);
    obj.insert("arrivalIndex"_L1, ld.arrivalIndex);
    return obj;
}

LiveData LiveData::fromJson(const QJsonObject &obj)
{
    LiveData ld;
    ld.trip = KPublicTransport::JourneySection::fromJson(obj.value("journey"_L1).toObject());
    ld.journeyTimestamp = QDateTime::fromString(obj.value("journeyTimestamp"_L1).toString(), Qt::ISODate);

    // backward compatibility for old journey storage format
    if (auto it = obj.find("departure"_L1); it != obj.end()) {
        const auto dep = KPublicTransport::Stopover::fromJson((*it).toObject());
        ld.trip.setDeparture(dep);
    }
    if (auto it = obj.find("arrival"_L1); it != obj.end()) {
        const auto arr = KPublicTransport::Stopover::fromJson((*it).toObject());
        ld.trip.setArrival(arr);
    }

    // backward compatibility for missing trip indexes
    ld.recoverIndexes();

    return ld;
}

QString LiveData::basePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/livedata/"_L1;
}

void LiveData::clearStorage()
{
    QDir(basePath()).removeRecursively();
}

void LiveData::recoverIndexes()
{
    if (trip.mode() != KPublicTransport::JourneySection::PublicTransport) {
        return;
    }
    departureIndex = std::max<qsizetype>(0, departureIndex);
    if (arrivalIndex < 0) {
        arrivalIndex = (qsizetype)trip.intermediateStops().size() + 1;
    }
}
