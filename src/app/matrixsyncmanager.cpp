/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matrixsyncmanager.h"

#include "documentmanager.h"
#include "livedatamanager.h"
#include "logging.h"
#include "matrixsynccontent.h"
#include "reservationmanager.h"
#include "transfermanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"

#if HAVE_MATRIX
#include <matrix/matrixmanager.h>

#include <Quotient/connection.h>
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
constexpr inline auto DocumentEventType = "org.kde.itinerary.document"_L1;

MatrixSyncManager::MatrixSyncManager(QObject *parent)
    : QObject(parent)
{
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
    qDebug() << tgId;
    if (!m_matrixMgr->connected()) {
        return; // TODO what do we do with changes while not connected?
    }

    const auto tg = m_tripGroupMgr->tripGroup(tgId);
    if (tg.name().isEmpty()) {
        return;
    }

    if (!tg.matrixRoomId().isEmpty()) {
        // TODO
    } else {
        auto job = m_matrixMgr->connection()->createRoom(Quotient::Connection::UnpublishRoom, {}, u"Itinerary Trip Sync"_s, tg.name(), {},  {}, {}, false, {}, {}, QJsonObject{{MatrixRoomTypeKey, ItineraryRoomType}});
        connect(job, &Quotient::CreateRoomJob::success, [this, tgId, job]() {
            qDebug() << "ROOM CREATED" << job->roomId();
            auto room = m_matrixMgr->connection()->room(job->roomId());
            if (!room) {
                return;
            }

            room->activateEncryption();
            room->addTag("m.lowpriority"_L1, 1.0);
            room->postPlainText("This is an automatically managed room for synchronizing KDE Itinerary trips. Your chat client is not supposed to display this room."_L1);

            auto tg = m_tripGroupMgr->tripGroup(tgId);
            tg.setMatrixRoomId(job->roomId());
            tg.setIsAutomaticallyGrouped(false);
            m_tripGroupMgr->updateTripGroup(tgId, tg);
            m_roomToTripGroupMap[job->roomId()] = tgId;

            for (const auto &elem : tg.elements()) {
                writeBatchToRoom(elem, room);
            }

            initRoom(room);
        });
    }
#endif
}

#if HAVE_MATRIX
void MatrixSyncManager::init()
{
    if (!m_matrixMgr || !m_tripGroupMgr || !m_docMgr || !m_ldm || !m_transferMgr) {
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
        qDebug() << "monitoring Matrix room" << tg.matrixRoomId();
        m_roomToTripGroupMap.insert(tg.matrixRoomId(), tgId);
    }

    if (m_matrixMgr->connection()) {
        initConnection(m_matrixMgr->connection());
    }
    connect(m_matrixMgr, &MatrixManager::connectionChanged, this, [this]() { initConnection(m_matrixMgr->connection()); });

    connect(m_ldm, &LiveDataManager::journeyUpdated, this, &MatrixSyncManager::liveDataChanged);

    connect(m_transferMgr, &TransferManager::transferAdded, this, [this](const auto &transfer) { transferChanged(transfer.reservationId(), transfer.alignment()); });
    connect(m_transferMgr, &TransferManager::transferChanged, this, [this](const auto &transfer) { transferChanged(transfer.reservationId(), transfer.alignment()); });
    connect(m_transferMgr, &TransferManager::transferRemoved, this, &MatrixSyncManager::transferChanged);
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

    if (m_matrixMgr->connected()) {
        reloadRooms();
    }
}

void MatrixSyncManager::initRoom(Quotient::Room *room)
{
    qDebug() << "setting up room connections for" << room->id();
    connect(room, &Quotient::Room::topicChanged, this, [this, room]() { roomTopicChanged(room); });
    connect(room, &Quotient::Room::aboutToAddNewMessages, this, [this, room](const auto &events) {
        for (const auto &event : events) {
            roomEvent(room, event.get());
        }
    });
    connect(room, &Quotient::Room::newFileTransfer, this, [](const QString &id, const QUrl &localFile) {
        qCDebug(Log) << "newFileTransfer" << id <<localFile;
    });
    connect(room, &Quotient::Room::fileTransferCompleted, this, [this, room](const QString &id, [[maybe_unused]] const QUrl &localFile, Quotient::FileSourceInfo info) {
        qCDebug(Log) << "fileTransferCompleted" << id <<m_pendingDocumentUploads;
        if (m_pendingDocumentUploads.contains(id)) {
            const auto url = Quotient::getUrlFromSourceInfo(info);
            qCDebug(Log) << "File upload completed" << id << url;
            m_pendingDocumentUploads.remove(id);

            QJsonObject file;
            Quotient::JsonObjectConverter<Quotient::EncryptedFileMetadata>::dumpTo(file, std::get<Quotient::EncryptedFileMetadata>(info));

            Quotient::StateEvent state(DocumentEventType, id, QJsonObject({
                {"url"_L1, url.toString()},
                {"file"_L1, file},
                {"metadata"_L1, KItinerary::JsonLdDocument::toJson(m_docMgr->documentInfo(id))}
            }));
            room->setState(state); // TODO error handling?
        }
    });
    connect(room, &Quotient::Room::fileTransferFailed, this, [](const QString &id, const QString &errorMsg) {
        // TODO error handling
        qCWarning(Log) << "file transfer failed:" << id <<errorMsg;
    });
    connect(room, &Quotient::Room::fileTransferProgress, this, [](const QString &id, qint64 progress, qint64 total) {
        // TODO
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
    qDebug() << room->id() << tg.name() << room->topic();
    if (room->topic() == tg.name()) {
        return;
    }

    tg.setName(room->topic());
    tg.setNameIsAutomatic(false);
    m_tripGroupMgr->updateTripGroup(it.value(), tg);
}

void MatrixSyncManager::roomEvent(Quotient::Room *room, const Quotient::RoomEvent *event)
{
    qDebug() << "NEW EVENT" << room->id() << event->contentJson();
    if (!event->isStateEvent()) {
        return;
    }
    const auto it = m_roomToTripGroupMap.find(room->id());
    if (it == m_roomToTripGroupMap.end()) {
        return;
    }

    const auto state = static_cast<const Quotient::StateEvent*>(event);
    if (event->matrixType() == MatrixSyncContent::ReservationEventType) {
        QScopedValueRollback recursionLock(m_recursionLock, true);
        TripGroupingBlocker blocker(m_tripGroupMgr);
        const auto resId = MatrixSyncContent::readBatch(state, m_tripGroupMgr->reservationManager());

        auto tg = m_tripGroupMgr->tripGroup(it.value());
        if (!resId.isEmpty() && !tg.elements().contains(resId)) {
            qCDebug(Log) << "adding reservation to trip group" << resId << it.value() << tg.elements();
            m_tripGroupMgr->addToGroup({resId}, it.value());
        }
        // resId.isEmpty() ie. deletion will be automatically propagated
    }

    if (event->matrixType() == DocumentEventType) {
        readDocumentFromStateEvent(state);
    }

    if (event->matrixType() == MatrixSyncContent::LiveDataEventType) {
        MatrixSyncContent::readLiveData(state, m_ldm);
    }

    if (event->matrixType() == MatrixSyncContent::TransferEventType) {
        MatrixSyncContent::readTransfer(state, m_transferMgr);
    }
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
    qDebug() << room->id();
    if (auto it = m_roomToTripGroupMap.find(room->id()); it != m_roomToTripGroupMap.end()) {
        const auto tgId = it.value();
        auto tg = m_tripGroupMgr->tripGroup(tgId);
        tg.setMatrixRoomId({});
        m_tripGroupMgr->updateTripGroup(tgId, tg);
        m_roomToTripGroupMap.erase(it);
    }
}

void MatrixSyncManager::tripGroupAdded(const QString &tgId)
{
    if (!m_autoSyncTrips || m_recursionLock) {
        return;
    }
    syncTripGroup(tgId);
}

void MatrixSyncManager::tripGroupChanged(const QString &tgId)
{
    if (m_recursionLock) {
        return;
    }

    qDebug() << tgId;
    if (!m_matrixMgr->connected()) { // TODO what do we do with changes while not connected?
        return;
    }

    const auto tg = m_tripGroupMgr->tripGroup(tgId);
    qDebug() << tg.matrixRoomId();
    if (tg.matrixRoomId().isEmpty()) {
        return; // not a synchronized room
    }

    auto room = m_matrixMgr->connection()->room(tg.matrixRoomId());
    if (!room) {
        qDebug() << "room not found?" << tg.matrixRoomId();
        return;
    }

    if (room->topic() != tg.name()) {
        qDebug() << room->id() << room->topic() << tg.name();
        room->setTopic(tg.name());
    }

    // TODO update group content
    // TODO this still needs proper state tracking
    const auto elems = tg.elements();
    for (const auto &resId : elems) {
        writeBatchToRoom(resId, room);
    }
    const auto states = room->currentState().eventsOfType(MatrixSyncContent::ReservationEventType);
    for (auto event : states) {
        if (!elems.contains(event->stateKey())) {
            writeBatchDeletionToRoom(event->stateKey(), room);
        }
    }
}

void MatrixSyncManager::tripGroupRemoved(const QString &tgId)
{
    if (m_recursionLock) {
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

void MatrixSyncManager::liveDataChanged(const QString &batchId)
{
    if (m_recursionLock) {
        return;
    }

    const auto tg = m_tripGroupMgr->tripGroupForReservation(batchId);
    if (tg.matrixRoomId().isEmpty()) {
        return;
    }

    auto room = m_matrixMgr->connection()->room(tg.matrixRoomId());
    if (!room) {
        qCDebug(Log) << "room not found?" << tg.matrixRoomId();
        return;
    }

    auto state = MatrixSyncContent::stateEventForLiveData(batchId);
    room->setState(*state);
}

void MatrixSyncManager::transferChanged(const QString &batchId, Transfer::Alignment alignment)
{
    if (m_recursionLock) {
        return;
    }

    const auto tg = m_tripGroupMgr->tripGroupForReservation(batchId);
    if (tg.matrixRoomId().isEmpty()) {
        return;
    }

    auto room = m_matrixMgr->connection()->room(tg.matrixRoomId());
    if (!room) {
        qCDebug(Log) << "room not found?" << tg.matrixRoomId();
        return;
    }

    auto state = MatrixSyncContent::stateEventForTransfer(batchId, alignment, m_transferMgr);
    room->setState(*state);
}

void MatrixSyncManager::createTripGroupFromRoom(Quotient::Room *room)
{
    QScopedValueRollback recursionLock(m_recursionLock, true);
    TripGroupingBlocker tgBlocker(m_tripGroupMgr);

    // load existing documents
    const auto documents = room->currentState().eventsOfType(DocumentEventType);
    for (auto event :documents) {
        readDocumentFromStateEvent(event);
    }

    // load existing content
    QStringList elems;
    const auto states = room->currentState().eventsOfType(MatrixSyncContent::ReservationEventType);
    for (auto event : states) {
        auto resId = MatrixSyncContent::readBatch(event, m_tripGroupMgr->reservationManager());
        if (!resId.isEmpty()) {
            elems.push_back(std::move(resId));
        }
    }
    const auto ldmStates = room->currentState().eventsOfType(MatrixSyncContent::LiveDataEventType);
    for (auto event : ldmStates) {
        MatrixSyncContent::readLiveData(event, m_ldm);
    }
    const auto transferStates = room->currentState().eventsOfType(MatrixSyncContent::TransferEventType);
    for (auto event :transferStates) {
        MatrixSyncContent::readTransfer(event, m_transferMgr);
    }

    // create group
    TripGroup tg;
    tg.setName(room->topic());
    tg.setMatrixRoomId(room->id());
    tg.setIsAutomaticallyGrouped(false);
    tg.setNameIsAutomatic(false);
    tg.setElements(elems);
    qCDebug(Log)  << "Creating trip group from Matrix" << tg.name() << room->id();
    const auto tgId = m_tripGroupMgr->createGroup(tg);
    m_roomToTripGroupMap.insert(room->id(), tgId);
}

void MatrixSyncManager::readDocumentFromStateEvent(const Quotient::StateEvent *event)
{
    const auto docId = event->stateKey();
    if (m_docMgr->hasDocument(docId) || m_pendingDocumentDownloads.contains(docId)) {
        return;
    }

    const auto url = QUrl(event->contentJson().value("url"_L1).toString());
    const auto metaData = KItinerary::JsonLdDocument::fromJsonSingular(event->contentJson().value("metadata"_L1).toObject());
    Quotient::EncryptedFileMetadata encryptionData;
    Quotient::JsonObjectConverter<Quotient::EncryptedFileMetadata>::fillFrom(event->contentJson().value("file"_L1).toObject(), encryptionData);
    m_pendingDocumentDownloads.insert(docId);
    auto job = m_matrixMgr->connection()->downloadFile(url, encryptionData);
    connect(job, &Quotient::DownloadFileJob::success, this, [this, job, metaData, docId]() {
        qCDebug(Log) << "Download suceeded" << docId;
        job->deleteLater();
        m_pendingDocumentDownloads.remove(docId);
        m_docMgr->addDocument(docId, metaData, job->targetFileName());
    });
    connect(job, &Quotient::DownloadFileJob::failure, this, [this, job, docId]() {
        qCDebug(Log) << "Download failed" << job->errorString() << docId;
        job->deleteLater();
        m_pendingDocumentDownloads.remove(docId);
    });
}

void MatrixSyncManager::writeBatchToRoom(const QString &batchId, Quotient::Room *room)
{
    if (!m_matrixMgr->connected()) {
        return;
    }

    const auto resIds = m_tripGroupMgr->reservationManager()->reservationsForBatch(batchId);
    for (const auto &resId : resIds) {
        const auto res = m_tripGroupMgr->reservationManager()->reservation(resId);
        const auto docIds = KItinerary::DocumentUtil::documentIds(res);
        qCDebug(Log) << docIds;
        for (const auto &docIdV : docIds) {
            const auto docId = docIdV.toString();
            qCDebug(Log) << docId << m_docMgr->hasDocument(docId);
            if (!docId.isEmpty() && m_docMgr->hasDocument(docId)) {
                writeDocumentToRoom(docId, room);
            }
        }
    }

    const auto state = MatrixSyncContent::stateEventForBatch(batchId, m_tripGroupMgr->reservationManager());
    room->setState(*state); // TODO error handling?
    qCDebug(Log) << "Set state event for" << batchId << room->id();

    if (m_ldm->journey(batchId).mode() != KPublicTransport::JourneySection::Invalid) {
        const auto state = MatrixSyncContent::stateEventForLiveData(batchId);
        room->setState(*state);
    }
    for (const auto align : { Transfer::Before, Transfer::After }) {
        const auto transfer = m_transferMgr->transfer(batchId, align);
        if (transfer.state() != Transfer::UndefinedState) {
            const auto state = MatrixSyncContent::stateEventForTransfer(batchId, align, m_transferMgr);
            room->setState(*state);
        }
    }
}

void MatrixSyncManager::writeBatchDeletionToRoom(const QString &batchId, Quotient::Room *room)
{
    if (!m_matrixMgr->connected()) {
        return;
    }

    const auto state = MatrixSyncContent::stateEventForDeletedBatch(batchId);
    room->setState(*state); // TODO error handling?
    qCDebug(Log) << "Removed reservation" << batchId << room->id();
}

void MatrixSyncManager::writeDocumentToRoom(const QString &docId, Quotient::Room *room)
{
    qCDebug(Log) << "writeDocumentToRoom" << docId << room->id() <<m_pendingDocumentUploads << room->currentState().contains(DocumentEventType, docId);
    if (m_pendingDocumentUploads.contains(docId)) { // upload already in progress
        return;
    }

    if (room->currentState().contains(DocumentEventType, docId)) { // already uploaded
        return;
    }

    qCDebug(Log) << "Starting file upload" << docId << QUrl::fromLocalFile(m_docMgr->documentFilePath(docId));
    m_pendingDocumentUploads.insert(docId);
    room->uploadFile(docId, QUrl::fromLocalFile(m_docMgr->documentFilePath(docId)));
    qDebug(Log) << "status:" << room->fileTransferInfo(docId).status;
}
#endif

#include "moc_matrixsyncmanager.cpp"
