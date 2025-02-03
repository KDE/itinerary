/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

namespace KPublicTransport
{
class JourneyReply;
class JourneySection;
class Manager;
class OnboardStatus;
}

class KNotification;

class PkPassManager;
class ReservationManager;

/** Handles querying live data sources for delays, etc. */
class LiveDataManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(KPublicTransport::Manager *publicTransportManager READ publicTransportManager CONSTANT)
public:
    explicit LiveDataManager(QObject *parent = nullptr);
    ~LiveDataManager() override;

    KPublicTransport::Manager *publicTransportManager() const;

    void setReservationManager(ReservationManager *resMgr);
    void setPkPassManager(PkPassManager *pkPassMgr);

    void setPollingEnabled(bool pollingEnabled);
    void setShowNotificationsOnLockScreen(bool enabled);

    KPublicTransport::Stopover arrival(const QString &resId) const;
    KPublicTransport::Stopover departure(const QString &resId) const;
    KPublicTransport::JourneySection journey(const QString &resId) const;

    /** Sets journey data for a given reservation, overwriting all existing data
     *  and not triggering notifications.
     *  Used to retain live data from alternative journey selections for example.
     *  @see applyJourney
     */
    Q_INVOKABLE void setJourney(const QString &resId, const KPublicTransport::JourneySection &journey);

    /** Applies new journey data for @p resId, retains still valid information not included
     *  in @p journey and triggers notifications if necessary.
     *  @see setJourney
     */
    void applyJourney(const QString &resId, const KPublicTransport::JourneySection &journey);

    /** Import a single LiveData element.
     *  Used by Importer.
     */
    void importData(const QString &resId, LiveData &&data);

    /** Trigger disruption notification, only exposed for testing here. */
    Q_INVOKABLE void showNotification(const QString &resId);

    /** Checks all applicable elements for updates. */
    Q_INVOKABLE void checkForUpdates();

    /** Checks the given batches for updates. */
    Q_INVOKABLE void checkForUpdates(const QStringList &batchIds);

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
    void tripQueryFailed(const QString &resId);

    void updateJourneyData(const KPublicTransport::JourneySection &journey, const QString &resId, const QVariant &res);

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

    LiveData &data(const QString &resId) const;

    void poll();
    /// @p force will bypass the check if the data is still up to date
    void pollForUpdates(bool force);
    void pollBatchForUpdates(const QString &batchId, bool force);
    void clearArrived();
    int nextPollTime() const;
    int nextPollTimeForReservation(const QString &resId) const;

    /** Poll cooldown in msecs (ie. delay the next poll on errors). */
    int pollCooldown(const QString &redId) const;

    /** Last time we queried any kind of departure information for this reservation batch. */
    QDateTime lastDeparturePollTime(const QString &batchId, const QVariant &res) const;

    /** Notifications handling for pkpass updates. */
    void pkPassUpdated(const QString &passId, const QStringList &changes);

    ReservationManager *m_resMgr;
    PkPassManager *m_pkPassMgr;
    KPublicTransport::Manager *m_ptMgr;
    std::vector<QString> m_reservations;
    mutable QHash<QString, LiveData> m_data;
    QHash<QString, QDateTime> m_lastPollAttempt; // poll cooldown on error
    QHash<QString, QPointer<KNotification>> m_notifications;
    bool m_showNotificationsOnLockScreen = false;

    QTimer m_pollTimer;
    KPublicTransport::OnboardStatus *m_onboardStatus = nullptr;

    // date/time overrides for unit testing
    friend class LiveDataManagerTest;
    QDateTime now() const;
    QDateTime m_unitTestTime;
};

#endif // LIVEDATAMANAGER_H
