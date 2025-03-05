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
    return journey.from().isEmpty() && journey.to().isEmpty();
}

LiveData LiveData::load(const QString &resId)
{
    LiveData ld;

    QFile f(basePath() + resId + ".json"_L1);
    if (!f.open(QFile::ReadOnly)) {
        return ld;
    }

    ld.journeyTimestamp = f.fileTime(QFile::FileModificationTime);
    ld.journey = KPublicTransport::JourneySection::fromJson(JsonIO::read(f.readAll()).toObject());
    return ld;
}

void LiveData::store(const QString &resId) const
{
    const auto path = basePath();
    QDir().mkpath(path);

    const QString fileName = path + resId + ".json"_L1;

    const auto obj = KPublicTransport::JourneySection::toJson(journey);
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
        file.setFileTime(journeyTimestamp, QFile::FileModificationTime);
        file.close();
    }
}

void LiveData::remove(const QString &resId)
{
    QFile::remove(basePath() + resId + ".json"_L1);
}

std::vector<QString> LiveData::listAll()
{
    std::vector<QString> ids;
    for (QDirIterator it(basePath(), QDir::Files); it.hasNext();) {
        it.next();
        ids.push_back(it.fileInfo().baseName());
    }
    return ids;
}

QJsonObject LiveData::toJson(const LiveData &ld)
{
    QJsonObject obj;
    obj.insert("journey"_L1, KPublicTransport::JourneySection::toJson(ld.journey));
    obj.insert("journeyTimestamp"_L1, ld.journeyTimestamp.toString(Qt::ISODate));
    return obj;
}

LiveData LiveData::fromJson(const QJsonObject &obj)
{
    LiveData ld;
    ld.journey = KPublicTransport::JourneySection::fromJson(obj.value("journey"_L1).toObject());
    ld.journeyTimestamp = QDateTime::fromString(obj.value("journeyTimestamp"_L1).toString(), Qt::ISODate);

    // backward compatibility
    if (auto it = obj.find("departure"_L1); it != obj.end()) {
        const auto dep = KPublicTransport::Stopover::fromJson((*it).toObject());
        ld.journey.setDeparture(dep);
    }
    if (auto it = obj.find("arrival"_L1); it != obj.end()) {
        const auto arr = KPublicTransport::Stopover::fromJson((*it).toObject());
        ld.journey.setArrival(arr);
    }

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
