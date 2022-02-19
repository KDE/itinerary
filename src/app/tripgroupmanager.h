/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TRIPGROUPMANAGER_H
#define TRIPGROUPMANAGER_H

#include <QObject>
#include <QHash>

class ReservationManager;
class TripGroup;

/** Trip group computation and persistence.
 *  This operates on multi-traveler batches as provided by ReservationManager.
 */
class TripGroupManager : public QObject
{
    Q_OBJECT
public:
    explicit TripGroupManager(QObject *parent = nullptr);
    ~TripGroupManager() override;

    void setReservationManager(ReservationManager *resMgr);

    QVector<QString> tripGroups() const;
    TripGroup tripGroup(const QString &id) const;
    QString tripGroupIdForReservation(const QString &resId) const;
    TripGroup tripGroupForReservation(const QString &resId) const;

    static void clear(); // for testing only!

    /** Deletes all elements in the trip group with Identifier @p id. */
    Q_INVOKABLE void removeReservationsInGroup(const QString &groupId);

Q_SIGNALS:
    void tripGroupAdded(const QString &id);
    void tripGroupChanged(const QString &id);
    void tripGroupRemoved(const QString &id);

private:
    friend class TripGroup;

    static QString basePath();
    void load();
    void removeTripGroup(const QString &groupId);

    void batchAdded(const QString &resId);
    void batchContentChanged(const QString &resId);
    void batchRenamed(const QString &oldBatchId, const QString &newBatchId);
    void batchRemoved(const QString &resId);

    void scanAll();
    void scanOne(std::vector<QString>::const_iterator beginIt);
    void checkConsistency();
    QString guessName(const TripGroup &g) const;
    QString guessDestinationFromLodging(const TripGroup &g) const;
    QString guessDestinationFromTransportTimeGap(const TripGroup &g) const;
    bool isRoundTrip(const TripGroup &g) const;

    ReservationManager *m_resMgr = nullptr;
    QHash<QString, TripGroup> m_tripGroups;
    QHash<QString, QString> m_reservationToGroupMap;

    std::vector<QString> m_reservations;

    struct ReservationNumberSearch {
        int type;
        QString resNum;
    };
    std::vector<ReservationNumberSearch> m_resNumSearch;
};

#endif // TRIPGROUPMANAGER_H
