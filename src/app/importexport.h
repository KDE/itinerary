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
    void exportHealthCertificates(const HealthCertificateManager *healthCertMgr);
    void exportLiveData();
    void exportLiveDataForBatch(const QString &batchId);
    void exportSettings();

private:
    KItinerary::File *m_file;
};

/** Data import handling. */
class Importer
{
public:
    explicit Importer(const KItinerary::File *file);

    int importReservations(ReservationManager *resMgr);
    int importPasses(PkPassManager *pkPassMgr);
    int importDocuments(DocumentManager *docMgr);
    int importTransfers(const ReservationManager *resMgr, TransferManager *transferMgr);
    int importTripGroups(TripGroupManager *tgMgr);
    int importFavoriteLocations(FavoriteLocationModel *favLocModel);
    int importPasses(PassManager *passMgr);
    int importHealthCertificates(HealthCertificateManager *healthCertMgr);
    int importLiveData(LiveDataManager *liveDataMgr);
    int importSettings();

private:
    const KItinerary::File *m_file;
    QHash<QString, QString> m_resIdMap;
};

#endif // IMPORTEXPORT_H
