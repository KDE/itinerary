/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef IMPORTEXPORT_H
#define IMPORTEXPORT_H

class DocumentManager;
class FavoriteLocationModel;
class HealthCertificateManager;
class LiveDataManager;
class PkPassManager;
class ReservationManager;
class TransferManager;

namespace KItinerary {
class File;
}

/** Data export handling. */
class Exporter
{
public:
    explicit Exporter(KItinerary::File *file);

    void exportReservations(const ReservationManager *resMgr);
    void exportPasses(const PkPassManager *pkPassMgr);
    void exportDocuments(const DocumentManager *docMgr);
    void exportTransfers(const ReservationManager *resMgr, const TransferManager *transferMgr);
    void exportFavoriteLocations(const FavoriteLocationModel *favLocModel);
    void exportHealthCertificates(const HealthCertificateManager *healthCertMgr);
    void exportLiveData();
    void exportSettings();

private:
    KItinerary::File *m_file;
};

/** Data import handling. */
class Importer
{
public:
    explicit Importer(const KItinerary::File *file);

    void importReservations(ReservationManager *resMgr);
    void importPasses(PkPassManager *pkPassMgr);
    void importDocuments(DocumentManager *docMgr);
    void importTransfers(const ReservationManager *resMgr, TransferManager *transferMgr);
    void importFavoriteLocations(FavoriteLocationModel *favLocModel);
    void importLiveData(LiveDataManager *liveDataMgr);
    void importSettings();

private:
    const KItinerary::File *m_file;
};

#endif // IMPORTEXPORT_H
