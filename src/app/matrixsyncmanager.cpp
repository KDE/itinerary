/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matrixsyncmanager.h"

#include "documentmanager.h"
#include "livedatamanager.h"
#include "logging.h"
#include "matrixsynccontent.h"
#include "pkpassmanager.h"
#include "reservationmanager.h"
#include "transfer.h"
#include "transfermanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"

#include <KItinerary/SortUtil>

#if HAVE_MATRIX
#include <matrix/matrixmanager.h>

#include <Quotient/connection.h>
#include <Quotient/csapi/room_state.h>
#include <Quotient/jobs/downloadfilejob.h>
#include <Quotient/room.h>
#endif

#include <KItinerary/DocumentUtil>
#include <KItinerary/JsonLdDocument>

#include <QJsonDocument>
#include <QJsonObject>
#include <QScopedValueRollback>
#include <QUrl>

using namespace Qt::Literals;

constexpr inline auto MatrixRoomTypeKey = "type"_L1;
constexpr inline auto ItineraryRoomType = "org.kde.itinerary.tripSync"_L1;

MatrixSyncManager::MatrixSyncManager(QObject *parent)
    : QObject(parent)
{
    // local changes going out
    connect(&m_outboundQueue, &MatrixSyncOutboundStateQueue::batchChanged, this, [this](const QString &batchId, const QString &tgId) {
        const auto room = roomForTripGroup(tgId);
        setState(room, MatrixSyncContent::stateEventForBatch(batchId, m_tripGroupMgr->reservationManager()));
    });
    connect(&m_outboundQueue, &MatrixSyncOutboundStateQueue::batchRemoved, this, [this](const QString &batchId, const QString &tgId) {
        const auto room = roomForTripGroup(tgId);
        setState(room, MatrixSyncContent::stateEventForDeletedBatch(batchId));
    });

    connect(&m_outboundQueue, &MatrixSyncOutboundStateQueue::liveDataChanged, this, [this](const QString &batchId) {
        const auto room = roomForBatch(batchId);
        setState(room, MatrixSyncContent::stateEventForLiveData(batchId));
    });

    connect(&m_outboundQueue, &MatrixSyncOutboundStateQueue::transferChanged, this, [this](const QString &batchId, Transfer::Alignment alignment) {
        const auto room = roomForBatch(batchId);
        setState(room, MatrixSyncContent::stateEventForTransfer(batchId, alignment, m_transferMgr));
    });

    connect(&m_outboundQueue, &MatrixSyncOutboundStateQueue::documentAdded, this, [this](const QString &docId, const QString &tgId) {
        const auto room = roomForTripGroup(tgId);
        setState(room, MatrixSyncContent::stateEventForDocument(docId, m_docMgr));
    });
    connect(&m_outboundQueue, &MatrixSyncOutboundStateQueue::pkPassChanged, this, [this](const QString &passId, const QString &tgId) {
        const auto room = roomForTripGroup(tgId);
        setState(room, MatrixSyncContent::stateEventForPkPass(passId));
    });
    connect(&m_outboundQueue, &MatrixSyncOutboundStateQueue::tripGroupAdded, this, &MatrixSyncManager::createRoomForTripGroup);
    connect(&m_outboundQueue, &MatrixSyncOutboundStateQueue::tripGroupChanged, this, &MatrixSyncManager::writeTripGroupChanges);


    // remote changes coming in
    connect(&m_inboundQueue, &MatrixSyncInboundStateQueue::downloadFile, this, [this](const MatrixSyncStateEvent &state) {
        auto job = m_matrixMgr->connection()->downloadFile(state.url(), state.encryptionData());
        connect(job, &Quotient::DownloadFileJob::success, this, [this, job]() {
            job->deleteLater();
            m_inboundQueue.setFileName(job->targetFileName());
        });
        connect(job, &Quotient::DownloadFileJob::failure, this, [this, job]() {
            job->deleteLater();
            m_inboundQueue.downloadFailed();
        });
    });

    connect(&m_inboundQueue, &MatrixSyncInboundStateQueue::reservationEvent, this, [this](const MatrixSyncStateEvent &state) {
        MatrixSyncLocalChangeLock recursionLock(&m_outboundQueue);
        TripGroupingBlocker blocker(m_tripGroupMgr);

        const auto &tgId = m_roomToTripGroupMap.value(state.roomId());
        if (MatrixSyncContent::isDeletedBatch(state)) {
            // only apply deletions if the trip group matches, as an extra safety measure e.g. against meanwhile split groups
            const auto tg = m_tripGroupMgr->tripGroup(tgId);
            if (tg.elements().contains(state.stateKey())) {
                qCDebug(Log) << "deleting reservation" << state.stateKey() << tgId;
                MatrixSyncContent::readBatch(state, m_tripGroupMgr->reservationManager());
            } else {
                qCDebug(Log) << "ignoring reservation deletion due to trip group mismatch" << state.stateKey() << tgId;
            }
        } else {
            const auto resId = MatrixSyncContent::readBatch(state, m_tripGroupMgr->reservationManager());
            // we can get here also without a matching trip group, when creating a new one
            // that is fine, the trip group will be created later below then
            if (!tgId.isEmpty()) {
                auto tg = m_tripGroupMgr->tripGroup(tgId);
                if (!resId.isEmpty() && !tg.elements().contains(resId)) {
                    qCDebug(Log) << "adding reservation to trip group" << resId << tgId << tg.elements();
                    m_tripGroupMgr->addToGroup({resId}, tgId);
                }
                // resId.isEmpty() ie. deletion will be automatically propagated
            } else {
                qCDebug(Log) << "could not find trip group for room" << state.roomId() << resId;
            }
        }
    });
    connect(&m_inboundQueue, &MatrixSyncInboundStateQueue::liveDataEvent, this, [this](const MatrixSyncStateEvent &state) {
        MatrixSyncLocalChangeLock recursionLock(&m_outboundQueue);
        MatrixSyncContent::readLiveData(state, m_ldm);
    });
    connect(&m_inboundQueue, &MatrixSyncInboundStateQueue::transferEvent, this, [this](const MatrixSyncStateEvent &state) {
        MatrixSyncLocalChangeLock recursionLock(&m_outboundQueue);
        MatrixSyncContent::readTransfer(state, m_transferMgr);
    });
    connect(&m_inboundQueue, &MatrixSyncInboundStateQueue::documentEvent, this, [this](const MatrixSyncStateEvent &state) {
        MatrixSyncLocalChangeLock recursionLock(&m_outboundQueue);
        MatrixSyncContent::readDocument(state, m_docMgr);
    });
    connect(&m_inboundQueue, &MatrixSyncInboundStateQueue::pkPassEvent, this, [this](const MatrixSyncStateEvent &state) {
        MatrixSyncLocalChangeLock recursionLock(&m_outboundQueue);
        MatrixSyncContent::readPkPass(state, m_pkPassMgr);
    });
}


MatrixSyncManager::~MatrixSyncManager() = default;

#if HAVE_MATRIX
void MatrixSyncManager::setTripGroupManager(TripGroupManager *tripGroupMgr)
{
    m_tripGroupMgr = tripGroupMgr;
    init();
}

void MatrixSyncManager::setMatrixManager(MatrixManager *mxMgr)
{
    m_matrixMgr = mxMgr;
    init();
}

void MatrixSyncManager::setDocumentManager(DocumentManager *docMgr)
{
    m_docMgr = docMgr;
    init();
}

void MatrixSyncManager::setLiveDataManager(LiveDataManager *ldm)
{
    m_ldm = ldm;
    init();
}

void MatrixSyncManager::setTransferManager(TransferManager *transferMgr)
{
    m_transferMgr = transferMgr;
    init();
}

void MatrixSyncManager::setPkPassManager(PkPassManager *pkPassMgr)
{
    m_pkPassMgr = pkPassMgr;
    init();
}

void MatrixSyncManager::setAutoSyncTrips(bool autoSync)
{
    m_autoSyncTrips = autoSync;
}

static bool isTripGroupSyncRoom(const Quotient::Room *room)
{
    if (!room) {
        return false;
    }

    const auto createEvent = room->currentState().get<Quotient::RoomCreateEvent>();
    if (!createEvent) {
        return false;
    }
    return createEvent && createEvent->contentJson().value(MatrixRoomTypeKey) == ItineraryRoomType;
}
#endif

void MatrixSyncManager::syncTripGroup(const QString &tgId)
{
#if HAVE_MATRIX
    m_outboundQueue.append(MatrixSyncOutboundStateQueue::TripGroupAdded, tgId);
#endif
}

#if HAVE_MATRIX
void MatrixSyncManager::init()
{
    if (!m_matrixMgr || !m_tripGroupMgr || !m_docMgr || !m_ldm || !m_transferMgr || !m_pkPassMgr) {
        return;
    }

    connect(m_tripGroupMgr, &TripGroupManager::tripGroupAdded, this, &MatrixSyncManager::tripGroupAdded);
    connect(m_tripGroupMgr, &TripGroupManager::tripGroupChanged, this, &MatrixSyncManager::tripGroupChanged);
    connect(m_tripGroupMgr, &TripGroupManager::tripGroupAboutToBeRemoved, this, &MatrixSyncManager::tripGroupRemoved);

    const auto tgIds = m_tripGroupMgr->tripGroups();
    for (const auto &tgId : tgIds) {
        const auto tg = m_tripGroupMgr->tripGroup(tgId);
        if (tg.matrixRoomId().isEmpty()) {
            continue;
        }
        qCDebug(Log) << "monitoring Matrix room" << tg.matrixRoomId();
        m_roomToTripGroupMap.insert(tg.matrixRoomId(), tgId);
    }

    if (m_matrixMgr->connection()) {
        initConnection(m_matrixMgr->connection());
    }
    connect(m_matrixMgr, &MatrixManager::connectionChanged, this, [this]() { initConnection(m_matrixMgr->connection()); });

    connect(m_tripGroupMgr->reservationManager(), &ReservationManager::batchChanged, &m_outboundQueue, [this](const auto &batchId) {
        m_outboundQueue.append(MatrixSyncOutboundStateQueue::BatchChange, batchId, m_tripGroupMgr->tripGroupIdForReservation(batchId));
    });
    connect(m_tripGroupMgr->reservationManager(), &ReservationManager::batchContentChanged, &m_outboundQueue, [this](const auto &batchId) {
        writeMissingDocuments(batchId, m_tripGroupMgr->tripGroupIdForReservation(batchId));
        m_outboundQueue.append(MatrixSyncOutboundStateQueue::BatchChange, batchId, m_tripGroupMgr->tripGroupIdForReservation(batchId));
    });

    connect(m_ldm, &LiveDataManager::journeyUpdated, &m_outboundQueue, [this](const auto &batchId) {
        m_outboundQueue.append(MatrixSyncOutboundStateQueue::LiveDataChange, batchId);
    });

    connect(m_transferMgr, &TransferManager::transferAdded, &m_outboundQueue, [this](const auto &transfer) {
        m_outboundQueue.append(MatrixSyncOutboundStateQueue::TransferChange, transfer.identifier());
    });
    connect(m_transferMgr, &TransferManager::transferChanged, &m_outboundQueue, [this](const auto &transfer) {
        m_outboundQueue.append(MatrixSyncOutboundStateQueue::TransferChange, transfer.identifier());
    });
    connect(m_transferMgr, &TransferManager::transferRemoved, &m_outboundQueue, [this](const auto &batchId, auto align) {
        m_outboundQueue.append(MatrixSyncOutboundStateQueue::TransferChange, Transfer::identifier(batchId, align));
    });

    connect(m_pkPassMgr, &PkPassManager::passUpdated, &m_outboundQueue, [this](const auto &passId) {
        m_outboundQueue.append(MatrixSyncOutboundStateQueue::PkPassChange, passId, tripGroupForPkPass(passId));
    });
}

void MatrixSyncManager::initConnection(Quotient::Connection *connection)
{
    qCDebug(Log) << "Matrix connected" <<connection;
    if (!connection) {
        return;
    }

    connect(connection, &Quotient::Connection::connected, this, &MatrixSyncManager::reloadRooms);
    connect(connection, &Quotient::Connection::joinedRoom, this, &MatrixSyncManager::joinedRoom);
    connect(connection, &Quotient::Connection::leftRoom, this, &MatrixSyncManager::deleteRoom);
    connect(connection, &Quotient::Connection::aboutToDeleteRoom, this, &MatrixSyncManager::deleteRoom);
    connect(connection, &Quotient::Connection::isOnlineChanged, this, [this]{
        if (m_matrixMgr->connection()->isOnline() == m_isOnline) {
            return;
        }
        m_isOnline = m_matrixMgr->connection()->isOnline();
        if (m_isOnline) {
            qCDebug(Log) << "Matrix connection online, starting replay";
            m_outboundQueue.retry();
        }
    });

    if (m_matrixMgr->connected()) {
        reloadRooms();
    }
}

void MatrixSyncManager::initRoom(Quotient::Room *room)
{
    qCDebug(Log) << "setting up room connections for" << room->id();
    connect(room, &Quotient::Room::topicChanged, this, [this, room]() { roomTopicChanged(room); });
    connect(room, &Quotient::Room::aboutToAddNewMessages, this, [this, room](const auto &events) {
        for (const auto &event : events) {
            roomEvent(room, event.get());
        }
    });
    connect(room, &Quotient::Room::newFileTransfer, this, [](const QString &id, const QUrl &localFile) {
        qCDebug(Log) << "newFileTransfer" << id <<localFile;
    });
    connect(room, &Quotient::Room::fileTransferCompleted, this, [this, room](const QString &id, [[maybe_unused]] const QUrl &localFile, const Quotient::FileSourceInfo &info) {
        qCDebug(Log) << "fileTransferCompleted" << id;
        if (!m_currentUpload || (*m_currentUpload).stateKey() != id) {
            qCDebug(Log) << "  not the transfer we were waiting on" << (m_currentUpload ? (*m_currentUpload).stateKey() : QString());
            return; // not one of ours
        }
        m_currentUpload->setFileInfo(info);
        if (!m_currentUpload->needsUpload()) {
            setState(room, std::move(*m_currentUpload));
        } else {
            qCWarning(Log) << "Something went wrong with upload state tracking" << m_currentUpload->stateKey();
        }
    });
    connect(room, &Quotient::Room::fileTransferFailed, this, [](const QString &id, const QString &errorMsg) {
        // TODO error handling
        qCWarning(Log) << "file transfer failed:" << id <<errorMsg;
    });
    connect(room, &Quotient::Room::fileTransferProgress, this, [](const QString &id, qint64 progress, qint64 total) {
        qCDebug(Log) << "file transfer progress" << id << progress << total;
    });
}

void MatrixSyncManager::reloadRooms()
{
    qCDebug(Log) << "reloading Matrix rooms";
    const auto rooms = m_matrixMgr->connection()->allRooms();
    for (const auto room : rooms) {
        reloadRoom(room);
    }
}

void MatrixSyncManager::reloadRoom(Quotient::Room *room)
{
    const auto it = m_roomToTripGroupMap.find(room->id());
    if (it == m_roomToTripGroupMap.end()) {
        if (!room->currentState().get<Quotient::RoomCreateEvent>()) {
            connect(room, &Quotient::Room::baseStateLoaded, this, [this, room]() { reloadRoom(room); }, Qt::SingleShotConnection);
            return;
        }
        if (!isTripGroupSyncRoom(room) || room->topic().isEmpty()) {
            return;
        }
        createTripGroupFromRoom(room);
    }

    initRoom(room);
}

void MatrixSyncManager::roomTopicChanged(Quotient::Room *room)
{
    const auto it = m_roomToTripGroupMap.find(room->id());
    if (it == m_roomToTripGroupMap.end()) {
        return;
    }

    auto tg = m_tripGroupMgr->tripGroup(it.value());
    qCDebug(Log) << room->id() << tg.name() << room->topic();
    if (room->topic() == tg.name()) {
        return;
    }

    tg.setName(room->topic());
    tg.setNameIsAutomatic(false);
    m_tripGroupMgr->updateTripGroup(it.value(), tg);
}

void MatrixSyncManager::roomEvent(Quotient::Room *room, const Quotient::RoomEvent *event)
{
    qCDebug(Log) << "NEW EVENT" << room->id() << event->contentJson() << event->roomId();
    if (!event->isStateEvent()) {
        return;
    }
    const auto it = m_roomToTripGroupMap.find(room->id());
    if (it == m_roomToTripGroupMap.end()) {
        return;
    }

    const auto state = static_cast<const Quotient::StateEvent*>(event);
    if (state->matrixType() == MatrixSync::DocumentEventType && m_docMgr->hasDocument(state->stateKey())) {
        return;
    }

    m_inboundQueue.append(*state, room->id());
}

void MatrixSyncManager::joinedRoom(Quotient::Room *room, Quotient::Room *prevRoom)
{
    if (room == prevRoom) {
        return;
    }
    if (m_roomToTripGroupMap.contains(room->id())) {
        qCWarning(Log) << "Joined room is already monitored?" << room->id();
        return;
    }

    if (isTripGroupSyncRoom(room)) {
        createTripGroupFromRoom(room);
        initRoom(room);
    }
}

void MatrixSyncManager::deleteRoom(Quotient::Room *room)
{
    if (auto it = m_roomToTripGroupMap.find(room->id()); it != m_roomToTripGroupMap.end()) {
        qCDebug(Log) << "trip sync room deleted" << room->id();
        const auto tgId = it.value();
        auto tg = m_tripGroupMgr->tripGroup(tgId);
        tg.setMatrixRoomId({});
        m_tripGroupMgr->updateTripGroup(tgId, tg);
        m_roomToTripGroupMap.erase(it);
    }
}

void MatrixSyncManager::tripGroupAdded(const QString &tgId)
{
    if (m_autoSyncTrips) {
        syncTripGroup(tgId);
    }
}

void MatrixSyncManager::tripGroupChanged(const QString &tgId)
{
    const auto tg = m_tripGroupMgr->tripGroup(tgId);
    if (tg.matrixRoomId().isEmpty()) {
        return; // not a synchronized room
    }
    m_outboundQueue.append(MatrixSyncOutboundStateQueue::TripGroupChanged, tgId);
}

void MatrixSyncManager::tripGroupRemoved(const QString &tgId)
{
    if (m_outboundQueue.isSuspended()) {
        return;
    }

    const auto tg = m_tripGroupMgr->tripGroup(tgId);
    if (tg.matrixRoomId().isEmpty()) {
        return;
    }
    auto it = m_roomToTripGroupMap.find(tg.matrixRoomId());
    if (it == m_roomToTripGroupMap.end()) {
        return;
    }

    auto room = m_matrixMgr->connection()->room(tg.matrixRoomId());
    if (!isTripGroupSyncRoom(room)) {
        return;
    }

    qCDebug(Log) << "synchronized trip group removed" << tgId << room->id();
    // TODO for shared rooms: post some kind of deletion event
    room->leaveRoom();
    m_matrixMgr->connection()->forgetRoom(tgId);
    m_roomToTripGroupMap.erase(it);
}

void MatrixSyncManager::createTripGroupFromRoom(Quotient::Room *room)
{
    MatrixSyncLocalChangeLock recursionLock(&m_outboundQueue);
    TripGroupingBlocker tgBlocker(m_tripGroupMgr);

    // load existing documents
    const auto documents = room->currentState().eventsOfType(MatrixSync::DocumentEventType);
    for (auto event : documents) {
        if (!m_docMgr->hasDocument(event->stateKey())) {
            m_inboundQueue.append(*event, room->id());
        }
    }

    // load existing content
    QStringList elems;
    const auto states = room->currentState().eventsOfType(MatrixSync::ReservationEventType);
    for (auto event : states) {
        m_inboundQueue.append(*event, room->id());
        elems.push_back(event->stateKey());
    }
    const auto ldmStates = room->currentState().eventsOfType(MatrixSync::LiveDataEventType);
    for (auto event : ldmStates) {
        m_inboundQueue.append(*event, room->id());
    }
    const auto transferStates = room->currentState().eventsOfType(MatrixSync::TransferEventType);
    for (auto event :transferStates) {
        m_inboundQueue.append(*event, room->id());
    }

    const auto pkPasses = room->currentState().eventsOfType(MatrixSync::PkPassEventType);
    for (auto pass : pkPasses) {
        m_inboundQueue.append(*pass, room->id());
    }

    // create group
    TripGroup tg;
    tg.setName(room->topic());
    tg.setMatrixRoomId(room->id());
    tg.setIsAutomaticallyGrouped(false);
    tg.setNameIsAutomatic(false);
    std::ranges::sort(elems, [this](const auto &lhs, const auto &rhs) {
        return KItinerary::SortUtil::isBefore(m_tripGroupMgr->reservationManager()->reservation(lhs), m_tripGroupMgr->reservationManager()->reservation(rhs));
    });
    tg.setElements(elems);
    qCDebug(Log)  << "Creating trip group from Matrix" << tg.name() << room->id() << tg.elements();
    const auto tgId = m_tripGroupMgr->createGroup(tg);
    m_roomToTripGroupMap.insert(room->id(), tgId);
}

void MatrixSyncManager::createRoomForTripGroup(const QString &tgId)
{
    const auto tg = m_tripGroupMgr->tripGroup(tgId);
    if (tg.name().isEmpty() || !tg.matrixRoomId().isEmpty()) {
        m_outboundQueue.replayNext();
        return;
    }

    auto job = m_matrixMgr->connection()->createRoom(Quotient::Connection::UnpublishRoom, {}, u"Itinerary Trip Sync"_s, tg.name(), {},  {}, {}, false, {}, {}, QJsonObject{{MatrixRoomTypeKey, ItineraryRoomType}});
    connect(job, &Quotient::CreateRoomJob::success, [this, tgId, job]() {
        qCDebug(Log) << "ROOM CREATED" << job->roomId();
        auto room = m_matrixMgr->connection()->room(job->roomId());
        if (room) {
            room->activateEncryption();
            room->addTag("m.lowpriority"_L1, 1.0);
            room->postText("This is an automatically managed room for synchronizing KDE Itinerary trips. Your chat client is not supposed to display this room."_L1);

            auto tg = m_tripGroupMgr->tripGroup(tgId);
            tg.setMatrixRoomId(job->roomId());
            tg.setIsAutomaticallyGrouped(false);
            m_tripGroupMgr->updateTripGroup(tgId, tg);
            m_roomToTripGroupMap[job->roomId()] = tgId;

            for (const auto &elem : tg.elements()) {
                writeBatchToRoom(elem, tgId);
            }

            initRoom(room);
        }
        m_outboundQueue.replayNext();
    });
}

void MatrixSyncManager::writeBatchToRoom(const QString &batchId, const QString &tgId)
{
    writeMissingDocuments(batchId, tgId);
    m_outboundQueue.append(MatrixSyncOutboundStateQueue::BatchChange, batchId, tgId);

    if (m_ldm->journey(batchId).mode() != KPublicTransport::JourneySection::Invalid) {
        m_outboundQueue.append(MatrixSyncOutboundStateQueue::LiveDataChange, batchId);
    }
    for (const auto align : { Transfer::Before, Transfer::After }) {
        const auto transfer = m_transferMgr->transfer(batchId, align);
        if (transfer.state() != Transfer::UndefinedState) {
            m_outboundQueue.append(MatrixSyncOutboundStateQueue::TransferChange, Transfer::identifier(batchId, align));
        }
    }
}

void MatrixSyncManager::writeMissingDocuments(const QString &batchId, const QString &tgId)
{
    const auto room = roomForTripGroup(tgId);
    const auto resIds = m_tripGroupMgr->reservationManager()->reservationsForBatch(batchId);
    for (const auto &resId : resIds) {
        const auto res = m_tripGroupMgr->reservationManager()->reservation(resId);
        const auto docIds = KItinerary::DocumentUtil::documentIds(res);
        for (const auto &docIdV : docIds) {
            const auto docId = docIdV.toString();
            if (!docId.isEmpty() && m_docMgr->hasDocument(docId) && missingStateEventInRoom(room, MatrixSync::DocumentEventType, docId)) {
                m_outboundQueue.append(MatrixSyncOutboundStateQueue::DocumentAdd, docId, tgId);
            }
        }
        if (const auto pkPassId = PkPassManager::passId(res); !pkPassId.isEmpty() && missingStateEventInRoom(room, MatrixSync::PkPassEventType, pkPassId)) {
            m_outboundQueue.append(MatrixSyncOutboundStateQueue::PkPassChange, pkPassId, tgId);
        }
    }
}

void MatrixSyncManager::writeTripGroupChanges(const QString &tgId)
{
    const auto tg = m_tripGroupMgr->tripGroup(tgId);
    const auto room = m_matrixMgr->connection()->room(tg.matrixRoomId());
    if (!room) {
        qCDebug(Log) << "room not found?" << tg.matrixRoomId();
        m_outboundQueue.replayNext();
        return;
    }

    if (room->topic() != tg.name()) {
        room->setTopic(tg.name());
    }

    // update group content
    const auto states = room->currentState().eventsOfType(MatrixSync::ReservationEventType);
    auto elems = tg.elements();
    for (auto event : states) {
        const auto it = std::ranges::find(elems, event->stateKey());
        if (it == elems.end()) {
            if (const auto state = MatrixSyncStateEvent::fromQuotient(*event, room->id()); state && !MatrixSyncContent::isDeletedBatch(*state)) {
                m_outboundQueue.append(MatrixSyncOutboundStateQueue::BatchRemove, event->stateKey(), tgId);
            }
        } else {
            elems.erase(it);
        }
    }
    for (const auto &batchId : elems) {
        writeBatchToRoom(batchId, tgId);
    }

    m_outboundQueue.replayNext();
}

Quotient::Room* MatrixSyncManager::roomForTripGroup(const QString &tgId) const
{
    const auto tg = m_tripGroupMgr->tripGroup(tgId);
    if (tg.matrixRoomId().isEmpty()) {
        return {};
    }

    auto room = m_matrixMgr->connection()->room(tg.matrixRoomId());
    if (!room) {
        qCDebug(Log) << "room not found?" << tg.matrixRoomId();
        return {};
    }

    return room;
}

Quotient::Room* MatrixSyncManager::roomForBatch(const QString &batchId) const
{
    return roomForTripGroup(m_tripGroupMgr->tripGroupIdForReservation(batchId));
}

void MatrixSyncManager::setState(Quotient::Room *room, MatrixSyncStateEvent &&state)
{
    if (!room) {
        qCDebug(Log) << "Can't set state event, got no room!" << state.type() << state.stateKey();
        m_outboundQueue.replayNext();
        return;
    }

    if (state.needsUpload()) {
        qCDebug(Log) << "Uploading file for local state change" << state.type() << state.stateKey();
        room->uploadFile(state.stateKey(), QUrl::fromLocalFile(state.fileName()));
        m_currentUpload = std::move(state);
        return;
    }

    qCDebug(Log) << "Setting state event for" << state.type() << state.stateKey() << "in room" << room->id();
    auto job = room->setState(state.toQuotient());
    connect(job, &Quotient::SetRoomStateWithKeyJob::finished, this, [this, job]() {
        if (!job->status().good()) {
            qCWarning(Log) << job->status();
            // for persistent errors retry wont help
            if (job->status().code == Quotient::BaseJob::StatusCode::NotFound) { // that's what most 41x errors are mapped to
                qCWarning(Log) << "considering error as persistent and skipping";
                m_outboundQueue.replayNext();
            }
        } else {
            m_inboundQueue.addKnownEventId(job->eventId());
            m_outboundQueue.replayNext();
        }
    });
}

bool MatrixSyncManager::missingStateEventInRoom(const Quotient::Room *room, QLatin1StringView type, const QString &stateKey) const
{
    if (!room) {
        return true;
    }

    return !room->currentState().contains(type, stateKey);
}

QString MatrixSyncManager::tripGroupForPkPass(const QString &pkPassId) const
{
    for (auto it = m_roomToTripGroupMap.begin(); it != m_roomToTripGroupMap.end(); ++it) {
        const auto tg = m_tripGroupMgr->tripGroup(it.value());
        const auto elems = tg.elements();
        for (const auto &batchId : elems) {
            const auto resIds = m_tripGroupMgr->reservationManager()->reservationsForBatch(batchId);
            for (const auto &resId : resIds) {
                if (PkPassManager::passId(m_tripGroupMgr->reservationManager()->reservation(resId)) == pkPassId) {
                    return it.value();
                }
            }
        }
    }
    return {};
}
#endif

#include "moc_matrixsyncmanager.cpp"
