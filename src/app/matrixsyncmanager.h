/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MATRIXSYNCMANAGER_H
#define MATRIXSYNCMANAGER_H

#include <config-itinerary.h>

#include <QHash>
#include <QObject>

class MatrixManager;
class TripGroupManager;

namespace Quotient {
class Connection;
class Room;
class RoomEvent;
}


/** Matrix-based trip synchronization. */
class MatrixSyncManager : public QObject
{
    Q_OBJECT
public:
    explicit MatrixSyncManager(QObject *parent = nullptr);
    ~MatrixSyncManager();

#if HAVE_MATRIX
    void setMatrixManager(MatrixManager *mxMgr);
    void setTripGroupManager(TripGroupManager *tripGroupMgr);
    void setAutoSyncTrips(bool autoSync);
#endif

    /** Trigger manual synchronization. */
    Q_INVOKABLE void syncTripGroup(const QString &tgId);

private:
#if HAVE_MATRIX
    void init();
    void initConnection(Quotient::Connection *connection);
    void initRoom(Quotient::Room *room);

    void reloadRooms();
    void updateRoom(Quotient::Room *room, Quotient::Room *prev);
    void deleteRoom(Quotient::Room *room);

    void roomTopicChanged(Quotient::Room *room);
    void roomEvent(Quotient::Room *room, const Quotient::RoomEvent *event);

    void tripGroupAdded(const QString &tgId);
    void tripGroupChanged(const QString &tgId);
    void tripGroupRemoved(const QString &tgId);

    MatrixManager *m_matrixMgr = nullptr;
    TripGroupManager *m_tripGroupMgr = nullptr;
    // map Matrix room ids to trip groups
    QHash<QString, QString> m_roomToTripGroupMap;
    bool m_autoSyncTrips = false;
#endif
};


#endif
