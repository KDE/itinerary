/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matrixsyncmanager.h"

#include "reservationmanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"

#include <matrix/matrixmanager.h>

#include <KItinerary/JsonLdDocument>

#include <Quotient/connection.h>
#include <Quotient/room.h>

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

using namespace Qt::Literals;

#if HAVE_MATRIX

MatrixSyncManager::MatrixSyncManager(QObject *parent)
    : QObject(parent)
{
}

MatrixSyncManager::~MatrixSyncManager() = default;

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
    qDebug() << "connected!";
    connect(connection, &Quotient::Connection::connected, this, &MatrixSyncManager::reloadRooms);
    connect(connection, &Quotient::Connection::invitedRoom, this, &MatrixSyncManager::updateRoom);
    connect(connection, &Quotient::Connection::joinedRoom, this, &MatrixSyncManager::updateRoom);
    connect(connection, &Quotient::Connection::leftRoom, this, &MatrixSyncManager::updateRoom);
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
    qDebug();
    const auto rooms = m_matrixMgr->connection()->allRooms();
    for (const auto &room : rooms) {
        const auto it = m_roomToTripGroupMap.find(room->id());
        if (it == m_roomToTripGroupMap.end()) {
            // TODO check for rooms with our type and create new group for that
            continue;
        }

        qDebug() << room->id();
        initRoom(room);
    }
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
    if (!event->isStateEvent() || event->matrixType() != "org.kde.itinerary.reservation"_L1) {
        return;
    }
    const auto it = m_roomToTripGroupMap.find(room->id());
    if (it == m_roomToTripGroupMap.end()) {
        return;
    }

    TripGroupingBlocker blocker(m_tripGroupMgr);
    const auto resJson = event->contentJson()["content"_L1].toString().toUtf8();
    const auto res = KItinerary::JsonLdDocument::fromJsonSingular(QJsonDocument::fromJson(resJson).object());
    // TODO use updateReservation for not relying on merging for larger changes!!
    const auto resId = m_tripGroupMgr->reservationManager()->addReservation(res, event->stateKey());

    auto tg = m_tripGroupMgr->tripGroup(it.value());
    if (!tg.elements().contains(resId)) {
        m_tripGroupMgr->addToGroup({resId}, it.value());
    }
}

void MatrixSyncManager::updateRoom(Quotient::Room *room, Quotient::Room *prevRoom)
{
    if (room == prevRoom) {
        return;
    }

    const auto it = m_roomToTripGroupMap.find(room->id());
    if (it == m_roomToTripGroupMap.end()) {
        if (const auto createEvent = room->currentState().get<Quotient::RoomCreateEvent>(); createEvent && createEvent->contentJson().value("type"_L1) == "org.kde.itinerary.tripSync"_L1) {
            const auto tgId = m_tripGroupMgr->createEmptyGroup(room->topic());
            auto tg = m_tripGroupMgr->tripGroup(tgId);
            tg.setMatrixRoomId(room->id());
            m_tripGroupMgr->updateTripGroup(tgId, tg);
        }
        return;
    }

    qDebug() << room->id();
    initRoom(room);
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
        auto job = m_matrixMgr->connection()->createRoom(Quotient::Connection::UnpublishRoom, {}, u"Itinerary Trip Sync"_s, tg.name(), {},  {}, {}, false, {}, {}, QJsonObject{{"type"_L1, "org.kde.itinerary.tripSync"_L1}});
        connect(job, &Quotient::CreateRoomJob::success, [this, tgId, job]() {
            qDebug() << "ROOM CREATED" << job->roomId();
            auto tg = m_tripGroupMgr->tripGroup(tgId);
            tg.setMatrixRoomId(job->roomId());
            m_tripGroupMgr->updateTripGroup(tgId, tg);
            m_roomToTripGroupMap[job->roomId()] = tgId;

            // TODO add all meanwhile added reservations
        });
    }
}

void MatrixSyncManager::tripGroupChanged(const QString &tgId)
{
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
    for (const auto &resId : tg.elements()) {
        const auto res = m_tripGroupMgr->reservationManager()->reservation(resId);

        Quotient::StateEvent state("org.kde.itinerary.reservation"_L1, resId, QJsonObject({
            {"content"_L1, QString::fromUtf8(QJsonDocument(KItinerary::JsonLdDocument::toJson(res)).toJson(QJsonDocument::Compact))}
        }));
        room->setState(state); // TODO error handling?
        qDebug() << "setting state event for" << resId;
    }
}

void MatrixSyncManager::tripGroupRemoved(const QString &tgId)
{
    qDebug() << tgId;
    const auto tg = m_tripGroupMgr->tripGroup(tgId);
    if (tg.matrixRoomId().isEmpty()) {
        return;
    }
    auto it = m_roomToTripGroupMap.find(tg.matrixRoomId());
    if (it == m_roomToTripGroupMap.end()) {
        return;
    }

    auto room = m_matrixMgr->connection()->room(tg.matrixRoomId());
    if (!room) {
        return;
    }
    if (const auto createEvent = room->currentState().get<Quotient::RoomCreateEvent>(); !createEvent || createEvent->contentJson().value("type"_L1) != "org.kde.itinerary.tripSync"_L1) {
        return;
    }

    // TODO for shared rooms: post some kind of deletion event
    room->leaveRoom();
    m_matrixMgr->connection()->forgetRoom(tgId);
    m_roomToTripGroupMap.erase(it);
}

#include "moc_matrixsyncmanager.cpp"
#endif
