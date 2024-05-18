/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "importexport.h"

#include "bundle-constants.h"
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

using namespace Qt::Literals::StringLiterals;

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
    auto t = transferMgr->transfer(batchId, Transfer::Before);
    if (t.state() != Transfer::UndefinedState) {
        m_file->addCustomData(BUNDLE_TRANSFER_DOMAIN, t.identifier(), QJsonDocument(Transfer::toJson(t)).toJson());
    }
    t = transferMgr->transfer(batchId, Transfer::After);
    if (t.state() != Transfer::UndefinedState) {
        m_file->addCustomData(BUNDLE_TRANSFER_DOMAIN, t.identifier(), QJsonDocument(Transfer::toJson(t)).toJson());
    }
}

void Exporter::exportFavoriteLocations(const FavoriteLocationModel* favLocModel)
{
    // export favorite locations
    if (favLocModel->rowCount() > 0) {
        m_file->addCustomData(BUNDLE_FAVORITE_LOCATION_DOMAIN, u"locations"_s,
                        QJsonDocument(FavoriteLocation::toJson(favLocModel->favoriteLocations())).toJson());
    }
}

void Exporter::exportPasses(const PassManager *passMgr)
{
    for (int i = 0; i < passMgr->rowCount(); ++i) {
        const auto idx = passMgr->index(i, 0);
        m_file->addCustomData(
            BUNDLE_PASS_DOMAIN,
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
            BUNDLE_HEALTH_CERTIFICATE_DOMAIN,
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

    m_file->addCustomData(BUNDLE_LIVE_DATA_DOMAIN, batchId, QJsonDocument(LiveData::toJson(ld)).toJson());
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
    m_file->addCustomData(BUNDLE_SETTINGS_DOMAIN, QStringLiteral("settings.ini"), f.readAll());
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
    const auto resIds = m_file->reservations();
    for (const auto &batchId : resIds) {
        auto t = Transfer::fromJson(QJsonDocument::fromJson(m_file->customData(BUNDLE_TRANSFER_DOMAIN, Transfer::identifier(batchId, Transfer::Before))).object());
        transferMgr->importTransfer(t);
        count += t.state() != Transfer::UndefinedState ? 1 : 0;
        t = Transfer::fromJson(QJsonDocument::fromJson(m_file->customData(BUNDLE_TRANSFER_DOMAIN, Transfer::identifier(batchId, Transfer::After))).object());
        transferMgr->importTransfer(t);
        count += t.state() != Transfer::UndefinedState ? 1 : 0;
    }
    return count;
}

int Importer::importFavoriteLocations(FavoriteLocationModel *favLocModel)
{
    auto favLocs = FavoriteLocation::fromJson(QJsonDocument::fromJson(m_file->customData(BUNDLE_FAVORITE_LOCATION_DOMAIN, u"locations"_s)).array());
    if (!favLocs.empty()) {
        favLocModel->setFavoriteLocations(std::move(favLocs));
    }
    return favLocs.size();
}

int Importer::importPasses(PassManager *passMgr)
{
    const auto ids = m_file->listCustomData(BUNDLE_PASS_DOMAIN);
    for (const auto &id : ids) {
        const auto data = m_file->customData(BUNDLE_PASS_DOMAIN, id);
        const auto pass = KItinerary::JsonLdDocument::fromJsonSingular(QJsonDocument::fromJson(data).object());
        passMgr->import(pass, id);
    }
    return ids.size();
}

int Importer::importHealthCertificates(HealthCertificateManager *healthCertMgr)
{
    const auto certIds = m_file->listCustomData(BUNDLE_HEALTH_CERTIFICATE_DOMAIN);
    for (const auto &certId : certIds) {
        healthCertMgr->importCertificate(m_file->customData(BUNDLE_HEALTH_CERTIFICATE_DOMAIN, certId));
    }
    return certIds.size();
}

int Importer::importLiveData(LiveDataManager *liveDataMgr)
{
    const auto ids = m_file->listCustomData(BUNDLE_LIVE_DATA_DOMAIN);
    for (const auto &id : ids) {
        const auto obj = QJsonDocument::fromJson(m_file->customData(BUNDLE_LIVE_DATA_DOMAIN, id)).object();
        auto ld = LiveData::fromJson(obj);
        ld.store(id);
        liveDataMgr->importData(id, std::move(ld));
    }
    return ids.size();
}

int Importer::importSettings()
{
    const auto iniData = m_file->customData(BUNDLE_SETTINGS_DOMAIN, QStringLiteral("settings.ini"));
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
