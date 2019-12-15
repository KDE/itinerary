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

class ReservationManager;
class TripGroupManager;

/** Manages Transfer objects, including creation, removal and persistence. */
class TransferManager : public QObject
{
    Q_OBJECT
public:
    explicit TransferManager(QObject *parent = nullptr);
    ~TransferManager();
    void setReservationManager(ReservationManager *resMgr);
    void setTripGroupManager(TripGroupManager *tgMgr);

    /** Returns the transfer for reservation @p resId with @p alignment. */
    Transfer transfer(const QString &resId, Transfer::Alignment alignment) const;

    // TODO discard method

    // for unit tests only
    void overrideCurrentDateTime(const QDateTime &dt);
    static void clear();

Q_SIGNALS:
    void transferAdded(const Transfer &transfer);
    void transferChanged(const Transfer &transfer);
    void transferRemoved(const QString &resId, Transfer::Alignment alignment);

private:
    void rescan();

    void checkReservation(const QString &resId);
    void checkReservation(const QString &resId, const QVariant &res, Transfer::Alignment alignment);
    void checkTransferBefore(const QString &resId, const QVariant &res, Transfer transfer);
    void checkTransferAfter(const QString &resId, const QVariant &res, Transfer transfer);
    void reservationRemoved(const QString &resId);
    void tripGroupChanged(const QString &tgId);

    bool isFirstInTripGroup(const QString &resId) const;
    bool isLastInTripGroup(const QString &resId) const;

    void addOrUpdateTransfer(Transfer t);
    void removeTransfer(const Transfer &t);

    Transfer readFromFile(const QString &resId, Transfer::Alignment alignment) const;
    void writeToFile(const Transfer &transfer) const;
    void removeFile(const QString &resId, Transfer::Alignment alignment) const;

    QDateTime currentDateTime() const;

    ReservationManager *m_resMgr = nullptr;
    TripGroupManager *m_tgMgr = nullptr;
    mutable QHash<QString, Transfer> m_transfers[2];
    QDateTime m_nowOverride;
};

#endif // TRANSFERMANAGER_H
