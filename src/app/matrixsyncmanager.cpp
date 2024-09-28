/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matrixsyncmanager.h"

#include "logging.h"
#include "reservationmanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"

#include <matrix/matrixmanager.h>

#include <KItinerary/JsonLdDocument>

#include <Quotient/connection.h>
#include <Quotient/room.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QScopedValueRollback>

using namespace Qt::Literals;

constexpr inline auto MatrixRoomTypeKey = "type"_L1;
constexpr inline auto ItineraryRoomType = "org.kde.itinerary.tripSync"_L1;
constexpr inline auto ReservationEventType = "org.kde.itinerary.reservation"_L1;

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
            room->postText("This is an automatically managed room for synchronizing KDE Itinerary trips. Your chat client is not supposed to display this room."_L1);

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
    if (!m_matrixMgr || !m_tripGroupMgr) {
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
    if (!event->isStateEvent() || event->matrixType() != ReservationEventType) {
        return;
    }
    const auto it = m_roomToTripGroupMap.find(room->id());
    if (it == m_roomToTripGroupMap.end()) {
        return;
    }

    QScopedValueRollback recursionLock(m_recursionLock, true);
    TripGroupingBlocker blocker(m_tripGroupMgr);
    const auto resId = readBatchFromStateEvent(static_cast<const Quotient::StateEvent*>(event));

    auto tg = m_tripGroupMgr->tripGroup(it.value());
    if (!resId.isEmpty() && !tg.elements().contains(resId)) {
        qCDebug(Log) << "adding reservation to trip group" << resId << it.value() << tg.elements();
        m_tripGroupMgr->addToGroup({resId}, it.value());
    }
    // resId.isEmpty() ie. deletion will be automatically propagated
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
    const auto states = room->currentState().eventsOfType(ReservationEventType);
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

void MatrixSyncManager::createTripGroupFromRoom(Quotient::Room *room)
{
    QScopedValueRollback recursionLock(m_recursionLock, true);
    TripGroupingBlocker tgBlocker(m_tripGroupMgr);

    // load existing content
    QStringList elems;
    const auto states = room->currentState().eventsOfType(ReservationEventType);
    for (auto event : states) {
        auto resId = readBatchFromStateEvent(event);
        if (!resId.isEmpty()) {
            elems.push_back(std::move(resId));
        }
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

QString MatrixSyncManager::readBatchFromStateEvent(const Quotient::StateEvent *event)
{
    const auto resJson = event->contentJson()["content"_L1].toString().toUtf8();
    if (resJson.isEmpty()) {
        // deleted
        m_tripGroupMgr->reservationManager()->removeBatch(event->stateKey());
        return {};
    }

    const auto res = KItinerary::JsonLdDocument::fromJsonSingular(QJsonDocument::fromJson(resJson).object());
    // TODO batch vs reservation separation!
    auto resId = event->stateKey();
    if (m_tripGroupMgr->reservationManager()->hasBatch(resId)) {
        qCDebug(Log) << "updating reservation" << resId;
        m_tripGroupMgr->reservationManager()->updateReservation(resId, res);
        qCDebug(Log) << "updated reservation" << resId;
    } else {
        qCDebug(Log) << "creating reservation" << resId;
        resId = m_tripGroupMgr->reservationManager()->addReservation(res, resId);
        qCDebug(Log) << "created reservation" << resId;
    }
    return resId;
}

void MatrixSyncManager::writeBatchToRoom(const QString &batchId, Quotient::Room *room)
{
    if (!m_matrixMgr->connected()) {
        return;
    }

    const auto res = m_tripGroupMgr->reservationManager()->reservation(batchId);
    Quotient::StateEvent state(ReservationEventType, batchId, QJsonObject({
        {"content"_L1, QString::fromUtf8(QJsonDocument(KItinerary::JsonLdDocument::toJson(res)).toJson(QJsonDocument::Compact))}
    }));
    room->setState(state); // TODO error handling?
    qCDebug(Log) << "Set state event for" << batchId << room->id();
}

void MatrixSyncManager::writeBatchDeletionToRoom(const QString &batchId, Quotient::Room *room)
{
    if (!m_matrixMgr->connected()) {
        return;
    }

    Quotient::StateEvent state(ReservationEventType, batchId, {});
    room->setState(state); // TODO error handling?
    qCDebug(Log) << "Removed reservation" << batchId << room->id();
}
#endif

#include "moc_matrixsyncmanager.cpp"
