/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "migrator.h"

#include "jsonio.h"
#include "livedata.h"
#include "logging.h"
#include "reservationmanager.h"

#include <KPublicTransport/Journey>
#include <KPublicTransport/Stopover>

#include <QDirIterator>
#include <QFile>
#include <QJsonObject>
#include <QSettings>
#include <QStandardPaths>

using namespace Qt::Literals;

void Migrator::run()
{
    QSettings settings;
    auto version = settings.value("Version", 0).toInt();
    qCDebug(Log) << "Migrating from version" << version;

    switch (version) {
        case 0:
            dropTripGroupExpandCollapseState();
            ++version;
            [[fallthrough]];
        case 1:
            moveLiveData();
            ++version;
            [[fallthrough]];
        case 2:
            recomputeBatchTimes();
            ++version;
            // add future updates here with [[fallthrough]]
            break;
        default:
            // already up to date
            qCDebug(Log) << "Already on current version, nothing to do.";
            return;
    }

    qCDebug(Log) << "Migration done to version" << version;
    settings.setValue("Version", version);
}

void Migrator::dropTripGroupExpandCollapseState()
{
    // remove leftover collapse/expand state from the old TripGroupProxyModel
    QSettings settings;
    settings.remove("TripGroupProxyState");
}

void Migrator::moveLiveData()
{
    // move live data from XDG cache to appdata, drop unused arrival/departure data
    // and implicitly convert to the new KPT::Journey storage format

    const QString legacyBasePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/publictransport/"_L1;
    assert(!legacyBasePath.isEmpty());

    QDir().mkpath(LiveData::basePath());
    for (QDirIterator batchIt(ReservationManager::batchesBasePath(), {u"*.json"_s}, QDir::Files); batchIt.hasNext();) {
        batchIt.next();
        const auto batchId = batchIt.fileInfo().baseName();

        // load and merge old entries
        KPublicTransport::JourneySection jny;
        QDateTime timestamp;
        if (QFile f(legacyBasePath + "journey/"_L1 + batchId + ".json"_L1); f.open(QFile::ReadOnly)) {
            jny = KPublicTransport::JourneySection::fromJson(JsonIO::read(f.readAll()).toObject());
            timestamp = f.fileTime(QFile::FileModificationTime);
        }
        if (QFile f(legacyBasePath + "departure/"_L1 + batchId + ".json"_L1); f.open(QFile::ReadOnly)) {
            const auto dep = KPublicTransport::Stopover::fromJson(JsonIO::read(f.readAll()).toObject());
            if (dep.scheduledDepartureTime().isValid()) {
                jny.setDeparture(dep);
            }
            if (!timestamp.isValid()) {
                timestamp = f.fileTime(QFile::FileModificationTime);
            }
        }
        if (QFile f(legacyBasePath + "arrival/"_L1 + batchId + ".json"_L1); f.open(QFile::ReadOnly)) {
            const auto arr = KPublicTransport::Stopover::fromJson(JsonIO::read(f.readAll()).toObject());
            if (arr.scheduledArrivalTime().isValid()) {
                jny.setArrival(arr);
            }
            if (!timestamp.isValid()) {
                timestamp = f.fileTime(QFile::FileModificationTime);
            }
        }

        if (jny.mode() == KPublicTransport::JourneySection::Invalid) {
            continue;
        }

        qCDebug(Log) << "migrating" << batchId;

        QFile f(LiveData::basePath() + batchId + ".json"_L1);
        if (!f.open(QFile::WriteOnly | QFile::Truncate)) {
            qCWarning(Log) << "Failed to open file:" << f.fileName() << f.errorString();
            break;
        }
        f.write(JsonIO::write(KPublicTransport::JourneySection::toJson(jny)));
        f.close();

        // mtime changes need to be done without content changes to take effect
        f.open(QFile::WriteOnly | QFile::Append);
        f.setFileTime(timestamp, QFile::FileModificationTime);
        f.close();
    }

    // delete all old files
    QDir(legacyBasePath).removeRecursively();
}

// force a batch time recompuation, as we used the wrong end time initially
void Migrator::recomputeBatchTimes()
{
    ReservationManager resMgr;

    for (QDirIterator it(ReservationManager::batchesBasePath(), QDir::NoDotAndDotDot | QDir::Files); it.hasNext();) {
        it.next();
        QFile batchFile(it.filePath());
        if (!batchFile.open(QFile::ReadOnly)) {
            qCWarning(Log) << "Failed to open batch file" << it.filePath() << batchFile.errorString();
            continue;
        }

        const auto batchId = it.fileInfo().baseName();
        auto batch = ReservationBatch::fromJson(JsonIO::read(batchFile.readAll()).toObject());
        resMgr.populateBatchTimes(batch);
        ReservationManager::storeBatch(batchId, batch);
    }
}
