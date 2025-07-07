/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MATRIXSYNCMANAGER_H
#define MATRIXSYNCMANAGER_H

#include <config-itinerary.h>

#include "matrixsynclocalstatequeue.h"
#include "transfer.h"

#include <QHash>
#include <QObject>
#include <QSet>

class DocumentManager;
class LiveDataManager;
class MatrixManager;
class PkPassManager;
class TransferManager;
class TripGroupManager;

namespace Quotient {
class Connection;
class Room;
class RoomEvent;
class StateEvent;
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
    void setDocumentManager(DocumentManager *docMgr);
    void setLiveDataManager(LiveDataManager *ldm);
    void setTransferManager(TransferManager *transferMgr);
    void setPkPassManager(PkPassManager *pkPassMgr);
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
    void reloadRoom(Quotient::Room *room);
    void joinedRoom(Quotient::Room *room, Quotient::Room *prev);
    void deleteRoom(Quotient::Room *room);

    void roomTopicChanged(Quotient::Room *room);
    void roomEvent(Quotient::Room *room, const Quotient::RoomEvent *event);

    void tripGroupAdded(const QString &tgId);
    void tripGroupChanged(const QString &tgId);
    void tripGroupRemoved(const QString &tgId);

    void pkPassChanged(const QString &passId);

    void createTripGroupFromRoom(Quotient::Room *room);
    void readDocumentFromStateEvent(const Quotient::StateEvent *event);
    void readPkPassFromStateEvent(const Quotient::StateEvent *event);

    void writeBatchToRoom(const QString &batchId, const QString &tgId);

    [[nodiscard]] Quotient::Room* roomForTripGroup(const QString &tgId) const;
    [[nodiscard]] Quotient::Room* roomForBatch(const QString &batchId) const;
    void setState(Quotient::Room *room, const Quotient::StateEvent &state);

    /** Brute force search for the trip group a pkpass belongs into.
     *  We might want something for efficient than that eventually.
     */
    [[nodiscard]] QString tripGroupForPkPass(const QString &pkPassId) const;

    MatrixSyncLocalStateQueue m_stateQueue;
    MatrixManager *m_matrixMgr = nullptr;
    TripGroupManager *m_tripGroupMgr = nullptr;
    DocumentManager *m_docMgr = nullptr;
    LiveDataManager *m_ldm = nullptr;
    TransferManager *m_transferMgr = nullptr;
    PkPassManager *m_pkPassMgr = nullptr;
    // map Matrix room ids to trip groups
    QHash<QString, QString> m_roomToTripGroupMap;
    QSet<QString> m_pendingDocumentDownloads;
    bool m_autoSyncTrips = false;
#endif
};


#endif
