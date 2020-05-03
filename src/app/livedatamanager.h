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

#include "livedata.h"

#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QPointer>
#include <QTimer>

#include <vector>

namespace KItinerary {
class TrainTrip;
}

namespace KPublicTransport {
class JourneySection;
class Manager;
class StopoverReply;
}

class KNotification;

class PkPassManager;
class ReservationManager;

/** Handles querying live data sources for delays, etc. */
class LiveDataManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(KPublicTransport::Manager* publicTransportManager READ publicTransportManager CONSTANT)
public:
    explicit LiveDataManager(QObject *parent = nullptr);
    ~LiveDataManager();

    KPublicTransport::Manager* publicTransportManager() const;

    void setReservationManager(ReservationManager *resMgr);
    void setPkPassManager(PkPassManager *pkPassMgr);

    void setPollingEnabled(bool pollingEnabled);
    void setShowNotificationsOnLockScreen(bool enabled);

    KPublicTransport::Stopover arrival(const QString &resId) const;
    KPublicTransport::Stopover departure(const QString &resId) const;
    KPublicTransport::JourneySection journey(const QString &resId) const;

    /** Sets journey data for a given reservation.
     *  Used to retain live data from alternative journey selections for example.
     */
    void setJourney(const QString &resId, const KPublicTransport::JourneySection &journey);

    /** Import a single LiveData element.
     *  Used by Importer.
     */
    void importData(const QString &resId, LiveData &&data);

    /** Trigger disruption notification, only exposed for testing here. */
    Q_INVOKABLE void showNotification(const QString &resId);

public Q_SLOTS:
    /** Checks all applicable elements for updates. */
    void checkForUpdates();

Q_SIGNALS:
    void arrivalUpdated(const QString &resId);
    void departureUpdated(const QString &resId);
    void journeyUpdated(const QString &resId);

private:
    bool isRelevant(const QString &resId) const;

    void batchAdded(const QString &resId);
    void batchChanged(const QString &resId);
    void batchRenamed(const QString &oldBatchId, const QString &newBatchId);
    void batchRemoved(const QString &resId);

    void checkReservation(const QVariant &res, const QString &resId);
    void stopoverQueryFinished(KPublicTransport::StopoverReply *reply, LiveData::Type type, const QString &resId);
    void stopoverQueryFinished(std::vector<KPublicTransport::Stopover> &&result, LiveData::Type type, const QString &resId);

    void updateStopoverData(const KPublicTransport::Stopover &stop, LiveData::Type type, const QString &resId, const QVariant &res);
    void showNotification(const QString &resId, const LiveData &ld);
    void fillNotification(KNotification *n, const LiveData &ld) const;
    void cancelNotification(const QString &resId);

    /** Best known departure time. */
    QDateTime departureTime(const QString &resId, const QVariant &res) const;
    /** Best known arrival time. */
    QDateTime arrivalTime(const QString &resId, const QVariant &res) const;
    /** Check if the trip @p res has departed, based on the best knowledge we have. */
    bool hasDeparted(const QString &resId, const QVariant &res) const;
    /** Check if the trip @p res has arrived, based on the best knowledge we have. */
    bool hasArrived(const QString &resId, const QVariant &res) const;

    LiveData& data(const QString &resId) const;

    void poll();
    /// @p force will bypass the check if the data is still up to date
    void pollForUpdates(bool force);
    int nextPollTime() const;
    int nextPollTimeForReservation(const QString &resId) const;

    /** Last time we queried any kind of departure information for this reservation batch. */
    QDateTime lastDeparturePollTime(const QString &batchId, const QVariant &res) const;

    /** Notifications handling for pkpass updates. */
    void pkPassUpdated(const QString &passId, const QStringList &changes);

    ReservationManager *m_resMgr;
    PkPassManager *m_pkPassMgr;
    KPublicTransport::Manager *m_ptMgr;
    std::vector<QString> m_reservations;
    mutable QHash <QString, LiveData> m_data;
    QHash<QString, QPointer<KNotification>> m_notifications;
    bool m_showNotificationsOnLockScreen = false;

    QTimer m_pollTimer;

    // date/time overrides for unit testing
    friend class LiveDataManagerTest;
    QDateTime now() const;
    QDateTime m_unitTestTime;
};

#endif // LIVEDATAMANAGER_H
