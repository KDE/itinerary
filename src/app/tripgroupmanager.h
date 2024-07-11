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

    [[nodiscard]] ReservationManager* reservationManager() const;
    void setReservationManager(ReservationManager *resMgr);
    void setTransferManager(TransferManager *transferMgr);

    /** Suspend automatic grouping, e.g. during mass operations.
     *  @see TripGroupingBlocker
     */
    void suspend();
    void resume();

    [[nodiscard]] std::vector<QString> tripGroups() const;
    Q_INVOKABLE [[nodiscard]] TripGroup tripGroup(const QString &id) const;
    [[nodiscard]] QString tripGroupIdForReservation(const QString &resId) const;
    [[nodiscard]] TripGroup tripGroupForReservation(const QString &resId) const;

    static void clear(); // for testing only!

    /** Update @p group, e.g. after editing its name. */
    Q_INVOKABLE void updateTripGroup(const QString &groupId, const TripGroup &group);

    /** Deletes all elements in the trip group with Identifier @p id. */
    Q_INVOKABLE void removeReservationsInGroup(const QString &groupId);

    /** Guesses a name for the given ordered set of reservations (potentially) forming a trip group. */
    Q_INVOKABLE [[nodiscard]] QString guessName(const QStringList &elements) const;

    /** Merge the two given trip groups into one.
     *  @returns the trip group id of the result.
     */
    Q_INVOKABLE [[nodiscard]] QString merge(const QString &tgId1, const QString &tgId2, const QString &newName);

Q_SIGNALS:
    void tripGroupAdded(const QString &id);
    void tripGroupChanged(const QString &id);
    void tripGroupRemoved(const QString &id);

private:
    friend class TripGroup;

    [[nodiscard]] static QString basePath();
    [[nodiscard]] static QString fileForGroup(QStringView tgId);
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
    [[nodiscard]] QString guessDestinationFromLodging(const QStringList &elements) const;
    [[nodiscard]] QString guessDestinationFromTransportTimeGap(const QStringList &elements) const;
    [[nodiscard]] bool isRoundTrip(const QStringList &elements) const;

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

    bool m_suspended : 1 = false;
    bool m_shouldScan : 1 = false;
};

/** RAII automatic trip grouping suspension. */
class TripGroupingBlocker
{
public:
    explicit inline TripGroupingBlocker(TripGroupManager *tgMgr)
        : m_tgMgr(tgMgr)
    {
        tgMgr->suspend();
    }
    inline ~TripGroupingBlocker()
    {
        m_tgMgr->resume();
    }
private:
    TripGroupManager *m_tgMgr;
};

#endif // TRIPGROUPMANAGER_H
