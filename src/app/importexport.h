/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef IMPORTEXPORT_H
#define IMPORTEXPORT_H

#include <QHash>

#include <vector>

class DocumentManager;
class FavoriteLocation;
class FavoriteLocationModel;
class HealthCertificateManager;
class LiveDataManager;
class PassManager;
class PkPassManager;
class ReservationManager;
class TransferManager;
class TripGroup;
class TripGroupManager;

namespace KItinerary
{
class File;
}

class QString;

/** Data export handling. */
class Exporter
{
public:
    explicit Exporter(KItinerary::File *file);

    void exportReservations(const ReservationManager *resMgr);
    void exportReservationBatch(const ReservationManager *resMgr, const QString &batchId);
    void exportPasses(const PkPassManager *pkPassMgr);
    void exportPkPass(const PkPassManager *pkPassMgr, const QString &passId);
    void exportDocuments(const DocumentManager *docMgr);
    void exportDocument(const DocumentManager *docMgr, const QString &docId);
    void exportTransfers(const ReservationManager *resMgr, const TransferManager *transferMgr);
    void exportTransfersForBatch(const QString &batchId, const TransferManager *transferMgr, std::vector<FavoriteLocation> &favoriteLocations);
    void exportTripGroups(const TripGroupManager *tripGroupMgr);
    void exportTripGroup(const QString &tripGroupId, const TripGroup &tg);
    void exportFavoriteLocations(const std::vector<FavoriteLocation> &favLocs);
    void exportPasses(const PassManager *passMgr);
    void exportPass(const QString &passId, const PassManager *passMgr);
    void exportHealthCertificates(const HealthCertificateManager *healthCertMgr);
    void exportLiveData();
    void exportLiveDataForBatch(const QString &batchId);
    void exportLocationSearchHistory();
    void exportPublicTransportAssets();
    void exportSettings();

private:
    KItinerary::File *m_file;
};

/** Data import handling. */
class Importer
{
public:
    explicit Importer(const KItinerary::File *file);
    [[nodiscard]] int formatVersion() const;

    qsizetype importReservations(ReservationManager *resMgr);
    qsizetype importPasses(PkPassManager *pkPassMgr);
    qsizetype importDocuments(DocumentManager *docMgr);
    qsizetype importTransfers(const ReservationManager *resMgr, TransferManager *transferMgr);
    qsizetype importTripGroups(TripGroupManager *tgMgr);
    qsizetype importFavoriteLocations(FavoriteLocationModel *favLocModel);
    qsizetype importPasses(PassManager *passMgr);
    qsizetype importHealthCertificates(HealthCertificateManager *healthCertMgr);
    qsizetype importLiveData(LiveDataManager *liveDataMgr);
    qsizetype importLocationSearchHistory();
    qsizetype importPublicTransportAssets();
    qsizetype importSettings();

private:
    const KItinerary::File *m_file;
    QHash<QString, QString> m_resIdMap;
};

#endif // IMPORTEXPORT_H
