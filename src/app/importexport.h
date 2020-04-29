/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef IMPORTEXPORT_H
#define IMPORTEXPORT_H

class DocumentManager;
class FavoriteLocationModel;
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
