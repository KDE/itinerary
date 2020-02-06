/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H

#include "transfer.h"

#include <QHash>
#include <QObject>

#include <cmath>

class FavoriteLocation;
class ReservationManager;
class TripGroupManager;

/** Manages Transfer objects, including creation, removal and persistence. */
class TransferManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float homeLatitude READ homeLatitude WRITE setHomeLatitude NOTIFY homeLocationChanged)
    Q_PROPERTY(float homeLongitude READ homeLongitude WRITE setHomeLongitude NOTIFY homeLocationChanged)

public:
    explicit TransferManager(QObject *parent = nullptr);
    ~TransferManager();
    void setReservationManager(ReservationManager *resMgr);
    void setTripGroupManager(TripGroupManager *tgMgr);

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

    // home coordinates, TODO might be better placed in a more generic location class, so we don't need to limit this to a single location
    float homeLatitude() const;
    void setHomeLatitude(float lat);
    float homeLongitude() const;
    void setHomeLongitude(float lon);
    bool hasHomeLocation() const;
    KPublicTransport::Location homeLocation() const;

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

    /**  Those two are used in both the automatical and manual code paths.
     *   @param transfer is filled with all required parameters, but not added yet
     *   @return @c true means the transfer can be added or updated (call addOrUpdateTransfer),
     *     @c false means the transfer should not be added/should be removed (call removeTransfer).
     */
    bool checkTransferBefore(const QString &resId, const QVariant &res, Transfer &transfer) const;
    bool checkTransferAfter(const QString &resId, const QVariant &res, Transfer &transfer) const;

    void reservationRemoved(const QString &resId);
    void tripGroupChanged(const QString &tgId);

    bool isFirstInTripGroup(const QString &resId) const;
    bool isLastInTripGroup(const QString &resId) const;

    void determineAnchorDeltaDefault(Transfer &transfer, const QVariant &res) const;

    void addOrUpdateTransfer(Transfer &t);
    void removeTransfer(const Transfer &t);

    Transfer readFromFile(const QString &resId, Transfer::Alignment alignment) const;
    void writeToFile(const Transfer &transfer) const;
    void removeFile(const QString &resId, Transfer::Alignment alignment) const;

    QDateTime currentDateTime() const;

    ReservationManager *m_resMgr = nullptr;
    TripGroupManager *m_tgMgr = nullptr;
    mutable QHash<QString, Transfer> m_transfers[2];
    QDateTime m_nowOverride;

    float m_homeLat = NAN;
    float m_homeLon = NAN;
};

#endif // TRANSFERMANAGER_H
