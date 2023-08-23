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
#include "passmanager.h"
#include "pkpassmanager.h"
#include "reservationmanager.h"
#include "transfer.h"
#include "transfermanager.h"

#include <KItinerary/File>
#include <KItinerary/JsonLdDocument>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QTemporaryFile>

static int copySettings(const QSettings *from, QSettings *to)
{
    const auto keys = from->allKeys();
    for (const auto &key : keys) {
        to->setValue(key, from->value(key));
    }
    return keys.size();
}

Exporter::Exporter(KItinerary::File *file)
    : m_file(file)
{
}

void Exporter::exportReservations(const ReservationManager *resMgr)
{
    for (const auto &batchId : resMgr->batches()) {
        exportReservationBatch(resMgr, batchId);
    }
}

void Exporter::exportReservationBatch(const ReservationManager *resMgr, const QString &batchId)
{
    const auto resIds = resMgr->reservationsForBatch(batchId);
    for (const auto &resId : resIds) {
        m_file->addReservation(resId, resMgr->reservation(resId));
    }
}

void Exporter::exportPasses(const PkPassManager *pkPassMgr)
{
    const auto passIds = pkPassMgr->passes();
    for (const auto &passId : passIds) {
        exportPkPass(pkPassMgr, passId);
    }
}

void Exporter::exportPkPass(const PkPassManager *pkPassMgr, const QString &passId)
{
    if (passId.isEmpty()) {
        return;
    }
    m_file->addPass(passId, pkPassMgr->rawData(passId));
}

void Exporter::exportDocuments(const DocumentManager *docMgr)
{
    const auto docIds = docMgr->documents();
    for (const auto &docId : docIds) {
        exportDocument(docMgr, docId);
    }
}

void Exporter::exportDocument(const DocumentManager *docMgr, const QString &docId)
{
    const auto fileName = docMgr->documentFilePath(docId);
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(Log) << "failed to open" << fileName << "for exporting" << file.errorString();
        return;
    }
    m_file->addDocument(docId, docMgr->documentInfo(docId), file.readAll());
}

void Exporter::exportTransfers(const ReservationManager *resMgr, const TransferManager *transferMgr)
{
    for (const auto &batchId : resMgr->batches()) {
        exportTransfersForBatch(batchId, transferMgr);
    }
}

void Exporter::exportTransfersForBatch(const QString &batchId, const TransferManager *transferMgr)
{
    const auto transferDomain = QStringLiteral("org.kde.itinerary/transfers");
    auto t = transferMgr->transfer(batchId, Transfer::Before);
    if (t.state() != Transfer::UndefinedState) {
        m_file->addCustomData(transferDomain, t.identifier(), QJsonDocument(Transfer::toJson(t)).toJson());
    }
    t = transferMgr->transfer(batchId, Transfer::After);
    if (t.state() != Transfer::UndefinedState) {
        m_file->addCustomData(transferDomain, t.identifier(), QJsonDocument(Transfer::toJson(t)).toJson());
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

void Exporter::exportPasses(const PassManager *passMgr)
{
    for (int i = 0; i < passMgr->rowCount(); ++i) {
        const auto idx = passMgr->index(i, 0);
        m_file->addCustomData(
            QStringLiteral("org.kde.itinerary/programs"),
            passMgr->data(idx, PassManager::PassIdRole).toString(),
            passMgr->data(idx, PassManager::PassDataRole).toByteArray()
        );
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
        exportLiveDataForBatch(id);
    }
}

void Exporter::exportLiveDataForBatch(const QString &batchId)
{
    const auto ld = LiveData::load(batchId);
    if (ld.isEmpty()) {
        return;
    }

    QJsonObject obj;
    obj.insert(QStringLiteral("departure"), KPublicTransport::Stopover::toJson(ld.departure));
    obj.insert(QStringLiteral("departureTimestamp"), ld.departureTimestamp.toString(Qt::ISODate));
    obj.insert(QStringLiteral("arrival"), KPublicTransport::Stopover::toJson(ld.arrival));
    obj.insert(QStringLiteral("arrivalTimestamp"), ld.arrivalTimestamp.toString(Qt::ISODate));
    obj.insert(QStringLiteral("journey"), KPublicTransport::JourneySection::toJson(ld.journey));
    obj.insert(QStringLiteral("journeyTimestamp"), ld.journeyTimestamp.toString(Qt::ISODate));

    m_file->addCustomData(QStringLiteral("org.kde.itinerary/live-data"), batchId, QJsonDocument(obj).toJson());
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

int Importer::importReservations(ReservationManager *resMgr)
{
    const auto resIds = m_file->reservations();
    for (const auto &resId : resIds) {
        resMgr->addReservation(m_file->reservation(resId), resId);
    }
    return resIds.size();
}

int Importer::importPasses(PkPassManager *pkPassMgr)
{
    const auto passIds = m_file->passes();
    for (const auto &passId : passIds) {
        pkPassMgr->importPassFromData(m_file->passData(passId));
    }
    return passIds.size();
}

int Importer::importDocuments(DocumentManager* docMgr)
{
    const auto docIds = m_file->documents();
    for (const auto &docId : docIds) {
        docMgr->addDocument(docId, m_file->documentInfo(docId), m_file->documentData(docId));
    }
    return docIds.size();
}

int Importer::importTransfers(const ReservationManager *resMgr, TransferManager *transferMgr)
{
    int count = 0;
    const auto transferDomain = QStringLiteral("org.kde.itinerary/transfers");
    const auto resIds = m_file->reservations();
    for (const auto &batchId : resIds) {
        auto t = Transfer::fromJson(QJsonDocument::fromJson(m_file->customData(transferDomain, Transfer::identifier(batchId, Transfer::Before))).object());
        transferMgr->importTransfer(t);
        count += t.state() != Transfer::UndefinedState ? 1 : 0;
        t = Transfer::fromJson(QJsonDocument::fromJson(m_file->customData(transferDomain, Transfer::identifier(batchId, Transfer::After))).object());
        transferMgr->importTransfer(t);
        count += t.state() != Transfer::UndefinedState ? 1 : 0;
    }
    return count;
}

int Importer::importFavoriteLocations(FavoriteLocationModel *favLocModel)
{
    auto favLocs = FavoriteLocation::fromJson(QJsonDocument::fromJson(m_file->customData(QStringLiteral("org.kde.itinerary/favorite-locations"), QStringLiteral("locations"))).array());
    if (!favLocs.empty()) {
        favLocModel->setFavoriteLocations(std::move(favLocs));
    }
    return favLocs.size();
}

int Importer::importPasses(PassManager *passMgr)
{
    const auto domain = QStringLiteral("org.kde.itinerary/programs");
    const auto ids = m_file->listCustomData(domain);
    for (const auto &id : ids) {
        const auto data = m_file->customData(domain, id);
        const auto pass = KItinerary::JsonLdDocument::fromJsonSingular(QJsonDocument::fromJson(data).object());
        passMgr->import(pass, id);
    }
    return ids.size();
}

int Importer::importHealthCertificates(HealthCertificateManager *healthCertMgr)
{
    const auto domain = QStringLiteral("org.kde.itinerary/health-certificates");
    const auto certIds = m_file->listCustomData(domain);
    for (const auto &certId : certIds) {
        healthCertMgr->importCertificate(m_file->customData(domain, certId));
    }
    return certIds.size();
}

int Importer::importLiveData(LiveDataManager *liveDataMgr)
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
    return ids.size();
}

int Importer::importSettings()
{
    const auto iniData = m_file->customData(QStringLiteral("org.kde.itinerary/settings"), QStringLiteral("settings.ini"));
    if (iniData.isEmpty()) {
        return 0;
    }

    QTemporaryFile tmp;
    if (!tmp.open()) {
        qCWarning(Log) << "Failed to open temporary file:" << tmp.errorString();
        return 0;
    }
    tmp.write(iniData);
    tmp.close();

    QSettings settings;
    const QSettings backup(tmp.fileName(), QSettings::IniFormat);
    return copySettings(&backup, &settings);
}
