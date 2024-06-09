/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matrixsyncmanager.h"

#include "tripgroup.h"
#include "tripgroupmanager.h"

#include <matrix/matrixmanager.h>

#include <Quotient/connection.h>
#include <Quotient/room.h>

#include <QDebug>

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
    connect(m_tripGroupMgr, &TripGroupManager::tripGroupRemoved, this, &MatrixSyncManager::tripGroupRemoved);

    const auto tgIds = m_tripGroupMgr->tripGroups();
    for (const auto &tgId : tgIds) {
        const auto tg = m_tripGroupMgr->tripGroup(tgId);
        if (tg.matrixRoomId().isEmpty()) {
            continue;
        }
        m_roomToTripGroupMap.insert(tg.matrixRoomId(), tgId);
    }

    if (m_matrixMgr->connection()) {
        initConnection(m_matrixMgr->connection());
    }
    connect(m_matrixMgr, &MatrixManager::connectionChanged, this, [this]() { initConnection(m_matrixMgr->connection()); });
}

void MatrixSyncManager::initConnection(Quotient::Connection *connection)
{
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
    connect(room, &Quotient::Room::topicChanged, this, [this, room]() { roomTopicChanged(room); });
}

void MatrixSyncManager::reloadRooms()
{
    qDebug();
    const auto rooms = m_matrixMgr->connection()->allRooms();
    for (const auto &room : rooms) {
        const auto it = m_roomToTripGroupMap.find(room->id());
        if (it == m_roomToTripGroupMap.end()) {
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

void MatrixSyncManager::updateRoom(Quotient::Room *room, Quotient::Room *prevRoom)
{
    if (room == prevRoom) {
        return;
    }

    const auto it = m_roomToTripGroupMap.find(room->id());
    if (it == m_roomToTripGroupMap.end()) {
        return;
    }

    qDebug() << room->id();
    initRoom(room);
}

void MatrixSyncManager::deleteRoom(Quotient::Room *room)
{
    qDebug() << room->id();
    // TODO
}

void MatrixSyncManager::tripGroupAdded(const QString &tgId)
{
    qDebug() << tgId;
    // TODO
}

void MatrixSyncManager::tripGroupChanged(const QString &tgId)
{
    if (!m_matrixMgr->connected()) { // TODO what do we do with changes while not connected?
        return;
    }

    const auto tg = m_tripGroupMgr->tripGroup(tgId);
    if (tg.matrixRoomId().isEmpty()) {
        return; // not a synchronized room
    }

    auto room = m_matrixMgr->connection()->room(tg.matrixRoomId());
    if (!room || room->topic() == tg.name()) {
        return;
    }

    qDebug() << room->id() << room->topic() << tg.name();
    room->setTopic(tg.name());
}

void MatrixSyncManager::tripGroupRemoved(const QString &tgId)
{
    qDebug() << tgId;
    // TODO
}

#include "moc_matrixsyncmanager.cpp"
#endif
