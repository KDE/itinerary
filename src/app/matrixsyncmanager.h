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
}

#if HAVE_MATRIX

/** Matrix-based trip synchronization. */
class MatrixSyncManager : public QObject
{
    Q_OBJECT
public:
    explicit MatrixSyncManager(QObject *parent = nullptr);
    ~MatrixSyncManager();

    void setMatrixManager(MatrixManager *mxMgr);
    void setTripGroupManager(TripGroupManager *tripGroupMgr);

private:
    void init();
    void initConnection(Quotient::Connection *connection);
    void initRoom(Quotient::Room *room);

    void reloadRooms();
    void updateRoom(Quotient::Room *room, Quotient::Room *prev);
    void deleteRoom(Quotient::Room *room);

    void roomTopicChanged(Quotient::Room *room);

    void tripGroupAdded(const QString &tgId);
    void tripGroupChanged(const QString &tgId);
    void tripGroupRemoved(const QString &tgId);

    MatrixManager *m_matrixMgr = nullptr;
    TripGroupManager *m_tripGroupMgr = nullptr;
    // map Matrix room ids to trip groups
    QHash<QString, QString> m_roomToTripGroupMap;
};

#endif

#endif
