/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TRIPGROUPMANAGER_H
#define TRIPGROUPMANAGER_H

#include "transfer.h"

#include <QObject>
#include <QHash>

class ReservationManager;
class TransferManager;
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
    void setTransferManager(TransferManager *transferMgr);

    [[nodiscard]] QList<QString> tripGroups() const;
    [[nodiscard]] TripGroup tripGroup(const QString &id) const;
    [[nodiscard]] QString tripGroupIdForReservation(const QString &resId) const;
    [[nodiscard]] TripGroup tripGroupForReservation(const QString &resId) const;

    static void clear(); // for testing only!

    /** Update @p group, e.g. after editing its name. */
    Q_INVOKABLE void updateTripGroup(const QString &groupId, const TripGroup &group);

    /** Deletes all elements in the trip group with Identifier @p id. */
    Q_INVOKABLE void removeReservationsInGroup(const QString &groupId);

Q_SIGNALS:
    void tripGroupAdded(const QString &id);
    void tripGroupChanged(const QString &id);
    void tripGroupRemoved(const QString &id);

private:
    friend class TripGroup;

    [[nodiscard]] static QString basePath();
    void load();
    void removeTripGroup(const QString &groupId);

    void batchAdded(const QString &resId);
    void batchContentChanged(const QString &resId);
    void batchRenamed(const QString &oldBatchId, const QString &newBatchId);
    void batchRemoved(const QString &resId);

    void transferChanged(const QString &resId, Transfer::Alignment alignment);

    void scanAll();
    void scanOne(std::vector<QString>::const_iterator beginIt);
    void checkConsistency();
    [[nodiscard]] QString guessName(const TripGroup &g) const;
    [[nodiscard]] QString guessDestinationFromLodging(const TripGroup &g) const;
    [[nodiscard]] QString guessDestinationFromTransportTimeGap(const TripGroup &g) const;
    [[nodiscard]] bool isRoundTrip(const TripGroup &g) const;

    /** Update begin/end times based on the current content.
     *  @returns @c true if the begin/end time changed.
     */
    bool recomputeTripGroupTimes(TripGroup &tg) const;

    ReservationManager *m_resMgr = nullptr;
    TransferManager *m_transferMgr = nullptr;
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
