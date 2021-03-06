/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "importexport.h"

#include "documentmanager.h"
#include "favoritelocationmodel.h"
#include "healthcertificatemanager.h"
#include "livedata.h"
#include "livedatamanager.h"
#include "logging.h"
#include "pkpassmanager.h"
#include "reservationmanager.h"
#include "transfer.h"
#include "transfermanager.h"

#include <KItinerary/File>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QTemporaryFile>

static void copySettings(const QSettings *from, QSettings *to)
{
    const auto keys = from->allKeys();
    for (const auto &key : keys) {
        to->setValue(key, from->value(key));
    }
}

Exporter::Exporter(KItinerary::File *file)
    : m_file(file)
{
}

void Exporter::exportReservations(const ReservationManager *resMgr)
{
    for (const auto &batchId : resMgr->batches()) {
        const auto resIds = resMgr->reservationsForBatch(batchId);
        for (const auto &resId : resIds) {
            m_file->addReservation(resId, resMgr->reservation(resId));
        }
    }
}

void Exporter::exportPasses(const PkPassManager *pkPassMgr)
{
    const auto passIds = pkPassMgr->passes();
    for (const auto &passId : passIds) {
        m_file->addPass(passId, pkPassMgr->rawData(passId));
    }
}

void Exporter::exportDocuments(const DocumentManager *docMgr)
{
    const auto docIds = docMgr->documents();
    for (const auto &docId : docIds) {
        const auto fileName = docMgr->documentFilePath(docId);
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly)) {
            qCWarning(Log) << "failed to open" << fileName << "for exporting" << file.errorString();
            continue;
        }
        m_file->addDocument(docId, docMgr->documentInfo(docId), file.readAll());
    }
}

void Exporter::exportTransfers(const ReservationManager *resMgr, const TransferManager *transferMgr)
{
    const auto transferDomain = QStringLiteral("org.kde.itinerary/transfers");
    for (const auto &batchId : resMgr->batches()) {
        auto t = transferMgr->transfer(batchId, Transfer::Before);
        if (t.state() != Transfer::UndefinedState) {
            m_file->addCustomData(transferDomain, t.identifier(), QJsonDocument(Transfer::toJson(t)).toJson());
        }
        t = transferMgr->transfer(batchId, Transfer::After);
        if (t.state() != Transfer::UndefinedState) {
            m_file->addCustomData(transferDomain, t.identifier(), QJsonDocument(Transfer::toJson(t)).toJson());
        }
    }
}

void Exporter::exportFavoriteLocations(const FavoriteLocationModel* favLocModel)
{
    // export favorite locations
    if (favLocModel->rowCount() > 0) {
        m_file->addCustomData(QStringLiteral("org.kde.itinerary/favorite-locations"), QStringLiteral("locations"),
                        QJsonDocument(FavoriteLocation::toJson(favLocModel->favoriteLocations())).toJson());
    }
}

void Exporter::exportHealthCertificates(const HealthCertificateManager *healthCertMgr)
{
    for (int i = 0; i < healthCertMgr->rowCount(); ++i) {
        const auto idx = healthCertMgr->index(i, 0);
        m_file->addCustomData(
            QStringLiteral("org.kde.itinerary/health-certificates"),
            healthCertMgr->data(idx, HealthCertificateManager::StorageIdRole).toString(),
            healthCertMgr->data(idx, HealthCertificateManager::RawDataRole).toByteArray());
    }
}

void Exporter::exportLiveData()
{
    for (const auto &id : LiveData::listAll()) {
        const auto ld = LiveData::load(id);

        QJsonObject obj;
        obj.insert(QStringLiteral("departure"), KPublicTransport::Stopover::toJson(ld.departure));
        obj.insert(QStringLiteral("departureTimestamp"), ld.departureTimestamp.toString(Qt::ISODate));
        obj.insert(QStringLiteral("arrival"), KPublicTransport::Stopover::toJson(ld.arrival));
        obj.insert(QStringLiteral("arrivalTimestamp"), ld.arrivalTimestamp.toString(Qt::ISODate));
        obj.insert(QStringLiteral("journey"), KPublicTransport::JourneySection::toJson(ld.journey));
        obj.insert(QStringLiteral("journeyTimestamp"), ld.journeyTimestamp.toString(Qt::ISODate));

        m_file->addCustomData(QStringLiteral("org.kde.itinerary/live-data"), id, QJsonDocument(obj).toJson());
    }
}

void Exporter::exportSettings()
{
    QTemporaryFile tmp;
    if (!tmp.open()) {
        qCWarning(Log) << "Failed to open temporary file:" << tmp.errorString();
        return;
    }

    QSettings settings;
    QSettings backup(tmp.fileName(), QSettings::IniFormat);
    tmp.close();
    copySettings(&settings, &backup);
    backup.sync();

    QFile f(backup.fileName());
    f.open(QFile::ReadOnly);
    m_file->addCustomData(QStringLiteral("org.kde.itinerary/settings"), QStringLiteral("settings.ini"), f.readAll());
}


Importer::Importer(const KItinerary::File *file)
    : m_file(file)
{
}

void Importer::importReservations(ReservationManager *resMgr)
{
    const auto resIds = m_file->reservations();
    for (const auto &resId : resIds) {
        resMgr->addReservation(m_file->reservation(resId));
    }
}

void Importer::importPasses(PkPassManager *pkPassMgr)
{
    const auto passIds = m_file->passes();
    for (const auto &passId : passIds) {
        pkPassMgr->importPassFromData(m_file->passData(passId));
    }
}

void Importer::importDocuments(DocumentManager* docMgr)
{
    const auto docIds = m_file->documents();
    for (const auto &docId : docIds) {
        docMgr->addDocument(docId, m_file->documentInfo(docId), m_file->documentData(docId));
    }
}

void Importer::importTransfers(const ReservationManager *resMgr, TransferManager *transferMgr)
{
    const auto transferDomain = QStringLiteral("org.kde.itinerary/transfers");
    for (const auto &batchId : resMgr->batches()) {
        auto t = Transfer::fromJson(QJsonDocument::fromJson(m_file->customData(transferDomain, Transfer::identifier(batchId, Transfer::Before))).object());
        transferMgr->importTransfer(t);
        t = Transfer::fromJson(QJsonDocument::fromJson(m_file->customData(transferDomain, Transfer::identifier(batchId, Transfer::After))).object());
        transferMgr->importTransfer(t);
    }
}

void Importer::importFavoriteLocations(FavoriteLocationModel *favLocModel)
{
    auto favLocs = FavoriteLocation::fromJson(QJsonDocument::fromJson(m_file->customData(QStringLiteral("org.kde.itinerary/favorite-locations"), QStringLiteral("locations"))).array());
    if (!favLocs.empty()) {
        favLocModel->setFavoriteLocations(std::move(favLocs));
    }
}

void Importer::importHealthCertificates(HealthCertificateManager *healthCertMgr)
{
    const auto domain = QStringLiteral("org.kde.itinerary/health-certificates");
    const auto certIds = m_file->listCustomData(domain);
    for (const auto &certId : certIds) {
        healthCertMgr->importCertificate(m_file->customData(domain, certId));
    }
}

void Importer::importLiveData(LiveDataManager *liveDataMgr)
{
    const auto ids = m_file->listCustomData(QStringLiteral("org.kde.itinerary/live-data"));
    for (const auto &id : ids) {
        const auto obj = QJsonDocument::fromJson(m_file->customData(QStringLiteral("org.kde.itinerary/live-data"), id)).object();

        LiveData ld;
        ld.departure = KPublicTransport::Stopover::fromJson(obj.value(QLatin1String("departure")).toObject());
        ld.departureTimestamp = QDateTime::fromString(obj.value(QLatin1String("departureTimestamp")).toString(), Qt::ISODate);
        ld.arrival = KPublicTransport::Stopover::fromJson(obj.value(QLatin1String("arrival")).toObject());
        ld.arrivalTimestamp = QDateTime::fromString(obj.value(QLatin1String("arrivalTimestamp")).toString(), Qt::ISODate);
        ld.journey = KPublicTransport::JourneySection::fromJson(obj.value(QLatin1String("journey")).toObject());
        ld.journeyTimestamp = QDateTime::fromString(obj.value(QLatin1String("journeyTimestamp")).toString(), Qt::ISODate);

        ld.store(id);
        liveDataMgr->importData(id, std::move(ld));
    }
}

void Importer::importSettings()
{
    QTemporaryFile tmp;
    if (!tmp.open()) {
        qCWarning(Log) << "Failed to open temporary file:" << tmp.errorString();
        return;
    }
    tmp.write(m_file->customData(QStringLiteral("org.kde.itinerary/settings"), QStringLiteral("settings.ini")));
    tmp.close();

    QSettings settings;
    const QSettings backup(tmp.fileName(), QSettings::IniFormat);
    copySettings(&backup, &settings);
}
