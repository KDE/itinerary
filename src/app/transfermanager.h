/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H

#include "transfer.h"

#include <KPublicTransport/JourneyRequest>

#include <QHash>
#include <QObject>

#include <cmath>

class FavoriteLocation;
class FavoriteLocationModel;
class LiveDataManager;
class ReservationManager;
class TripGroupManager;

/** Manages Transfer objects, including creation, removal and persistence. */
class TransferManager : public QObject
{
    Q_OBJECT

public:
    explicit TransferManager(QObject *parent = nullptr);
    ~TransferManager() override;
    void setReservationManager(ReservationManager *resMgr);
    void setTripGroupManager(TripGroupManager *tgMgr);
    void setFavoriteLocationModel(FavoriteLocationModel *favLocModel);
    void setLiveDataManager(LiveDataManager *liveDataMgr);

    void setAutoAddTransfers(bool enabled);
    void setAutoFillTransfers(bool enabled);

    /** Returns the transfer for reservation @p resId with @p alignment. */
    Transfer transfer(const QString &resId, Transfer::Alignment alignment) const;

    /** Applies the given @p journey to @p transfer. */
    Q_INVOKABLE void setJourneyForTransfer(Transfer transfer, const KPublicTransport::Journey &journey);
    /** Applies the given @p favoriteLocation to the floating end of @p transfer. */
    Q_INVOKABLE Transfer setFavoriteLocationForTransfer(Transfer transfer, const FavoriteLocation &favoriteLocation);
    /** Discard the given @p transfer. */
    Q_INVOKABLE void discardTransfer(Transfer transfer);

    /** Checks if a transfer can be added before/after the given reservation.
     *  This is used to manual inserts, and might allow more inserts than the automatic code would perform.
     */
    Q_INVOKABLE bool canAddTransfer(const QString &resId, Transfer::Alignment alignment) const;
    /** Explicitly add a transfer before/after the given reservation. */
    Q_INVOKABLE Transfer addTransfer(const QString &resId, Transfer::Alignment alignment);

    /** Create a JourneyRequest for a given @p transfer. */
    Q_INVOKABLE KPublicTransport::JourneyRequest journeyRequestForTransfer(const Transfer &transfer) const;

    void importTransfer(const Transfer &transfer);

    // for unit tests only
    void overrideCurrentDateTime(const QDateTime &dt);
    static void clear();

Q_SIGNALS:
    void transferAdded(const Transfer &transfer);
    void transferChanged(const Transfer &transfer);
    void transferRemoved(const QString &resId, Transfer::Alignment alignment);

    void homeLocationChanged();

private:
    void rescan(bool force = false);

    void checkReservation(const QString &resId);
    void checkReservation(const QString &resId, const QVariant &res, Transfer::Alignment alignment);

    enum CheckTransferResult {
        ShouldAutoAdd, /// transfer should be added automatically
        CanAddManually, /// transfer is possible, but should only be added manually
        ShouldRemove /// invalid transfer, cannot be added or should be removed
    };
    /**  Those two are used in both the automatical and manual code paths.
     *   @param transfer is filled with all required parameters, but not added yet
     */
    CheckTransferResult checkTransferBefore(const QString &resId, const QVariant &res, Transfer &transfer) const;
    CheckTransferResult checkTransferAfter(const QString &resId, const QVariant &res, Transfer &transfer) const;

    void reservationRemoved(const QString &resId);
    void tripGroupChanged(const QString &tgId);

    bool isFirstInTripGroup(const QString &resId) const;
    bool isLastInTripGroup(const QString &resId) const;
    bool isNotInTripGroup(const QString &resId) const;

    void determineAnchorDeltaDefault(Transfer &transfer, const QVariant &res) const;

    /** Return the transfer anchor time before/after the reservation @p res. */
    QDateTime anchorTimeBefore(const QString &resId, const QVariant &res) const;
    QDateTime anchorTimeAfter(const QString &resId, const QVariant &res) const;

    static KPublicTransport::Location locationFromFavorite(const FavoriteLocation &favLoc);
    /** Pick the best favorite location for a given transfer. */
    FavoriteLocation pickFavorite(const QVariant &anchoredLoc, const QString &resId, Transfer::Alignment alignment) const;

    void addOrUpdateTransfer(Transfer &t);
    void removeTransfer(const Transfer &t);

    Transfer readFromFile(const QString &resId, Transfer::Alignment alignment) const;
    void writeToFile(const Transfer &transfer) const;
    void removeFile(const QString &resId, Transfer::Alignment alignment) const;

    void autoFillTransfer(Transfer &t);

    QDateTime currentDateTime() const;

    ReservationManager *m_resMgr = nullptr;
    TripGroupManager *m_tgMgr = nullptr;
    FavoriteLocationModel *m_favLocModel = nullptr;
    LiveDataManager *m_liveDataMgr = nullptr;
    mutable QHash<QString, Transfer> m_transfers[2];
    QDateTime m_nowOverride;
    bool m_autoAddTransfers = true;
    bool m_autoFillTransfers = false;
};

#endif // TRANSFERMANAGER_H
