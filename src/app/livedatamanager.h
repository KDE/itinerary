/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

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

#ifndef LIVEDATAMANAGER_H
#define LIVEDATAMANAGER_H

#include <QHash>
#include <QObject>

#include <memory>
#include <vector>

namespace KItinerary {
class TrainTrip;
}

namespace KPublicTransport {
class Departure;
class Manager;
}

class PkPassManager;
class ReservationManager;

/** Handles querying live data sources for delays, etc. */
class LiveDataManager : public QObject
{
    Q_OBJECT
public:
    explicit LiveDataManager(QObject *parent = nullptr);
    ~LiveDataManager();

    void setReservationManager(ReservationManager *resMgr);
    void setPkPassManager(PkPassManager *pkPassMgr);

    Q_INVOKABLE QVariant arrival(const QString &resId);
    Q_INVOKABLE QVariant departure(const QString &resId);

public slots:
    /** Checks all applicable elements for updates. */
    void checkForUpdates();

signals:
    void arrivalUpdated(const QString &resId);
    void departureUpdated(const QString &resId);

private:
    bool isRelevant(const QString &resId) const;

    void reservationAdded(const QString &resId);
    void reservationChanged(const QString &resId);
    void reservationRemoved(const QString &resId);

    void checkTrainTrip(const KItinerary::TrainTrip &trip, const QString &resId);

    void updateArrivalData(const KPublicTransport::Departure &arr, const QString &resId);
    void updateDepartureData(const KPublicTransport::Departure &dep, const QString &resId);

    void storePublicTransportData(const QString &resId, const KPublicTransport::Departure &dep, const QString &type) const;
    void loadPublicTransportData();
    void loadPublicTransportData(const QString &prefix, QHash<QString, KPublicTransport::Departure>& data) const;

    ReservationManager *m_resMgr;
    PkPassManager *m_pkPassMgr;
    std::unique_ptr<KPublicTransport::Manager> m_ptMgr;
    std::vector<QString> m_reservations;
    QHash <QString, KPublicTransport::Departure> m_arrivals;
    QHash <QString, KPublicTransport::Departure> m_departures;
};

#endif // LIVEDATAMANAGER_H
