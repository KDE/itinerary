// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "matrixroomsmodel.h"

#include <Quotient/room.h>

using namespace Quotient;

Q_DECLARE_METATYPE(Quotient::JoinState)

MatrixRoomsModel::MatrixRoomsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void MatrixRoomsModel::setConnection(Connection *connection)
{
    if (connection == m_connection) {
        return;
    }
    if (m_connection) {
        m_connection->disconnect(this);
    }
    if (!connection) {
        m_connection = nullptr;
        beginResetModel();
        m_rooms.clear();
        endResetModel();
        return;
    }

    m_connection = connection;

    for (Room *room : std::as_const(m_rooms)) {
        room->disconnect(this);
    }

    connect(connection, &Connection::connected, this, &MatrixRoomsModel::doResetModel);
    connect(connection, &Connection::invitedRoom, this, &MatrixRoomsModel::updateRoom);
    connect(connection, &Connection::joinedRoom, this, &MatrixRoomsModel::updateRoom);
    connect(connection, &Connection::leftRoom, this, &MatrixRoomsModel::updateRoom);
    connect(connection, &Connection::aboutToDeleteRoom, this, &MatrixRoomsModel::deleteRoom);
    connect(connection, &Connection::directChatsListChanged, this, [this, connection](Quotient::DirectChatsMap additions, Quotient::DirectChatsMap removals) {
        auto refreshRooms = [this, &connection](Quotient::DirectChatsMap rooms) {
            for (const QString &roomID : std::as_const(rooms)) {
                auto room = connection->room(roomID);
                if (room) {
                    refresh(room);
                }
            }
        };

        refreshRooms(std::move(additions));
        refreshRooms(std::move(removals));
    });

    doResetModel();

    Q_EMIT connectionChanged();
}

void MatrixRoomsModel::doResetModel()
{
    beginResetModel();
    m_rooms.clear();
    const auto rooms = m_connection->allRooms();
    for (const auto &room : rooms) {
        doAddRoom(room);
    }
    endResetModel();
}

Room *MatrixRoomsModel::roomAt(int row) const
{
    return m_rooms.at(row);
}

void MatrixRoomsModel::doAddRoom(Room *room)
{
    if (room) {
        if (!room->successorId().isEmpty()) {
            return;
        }
        m_rooms.append(room);
        connectRoomSignals(room);
    } else {
        Q_ASSERT(false);
    }
}

void MatrixRoomsModel::connectRoomSignals(Room *room)
{
    connect(room, &Room::displaynameChanged, this, [this, room] {
        refresh(room);
    });
    connect(room, &Room::avatarChanged, this, [this, room] {
        refresh(room, {AvatarRole, AvatarImageRole});
    });
}

void MatrixRoomsModel::updateRoom(Room *room, Room *prev)
{
    if (prev == room) {
        refresh(room);
        return;
    }
    const auto newRoom = room;
    const auto it = std::find_if(m_rooms.begin(), m_rooms.end(), [=](const Room *r) {
        return r == prev || r == newRoom;
    });
    if (it != m_rooms.end()) {
        const int row = it - m_rooms.begin();
        // There's no guarantee that prev != newRoom
        if (*it == prev && *it != newRoom) {
            prev->disconnect(this);
            m_rooms.replace(row, newRoom);
            connectRoomSignals(newRoom);
        }
        Q_EMIT dataChanged(index(row), index(row));
    } else {
        beginInsertRows(QModelIndex(), m_rooms.count(), m_rooms.count());
        doAddRoom(newRoom);
        endInsertRows();
    }
}

void MatrixRoomsModel::deleteRoom(Room *room)
{
    const auto it = std::find(m_rooms.begin(), m_rooms.end(), room);
    if (it == m_rooms.end()) {
        return;
    }
    const int row = it - m_rooms.begin();
    beginRemoveRows(QModelIndex(), row, row);
    m_rooms.erase(it);
    endRemoveRows();
}

int MatrixRoomsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_rooms.count();
}

QVariant MatrixRoomsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.row() >= m_rooms.count()) {
        return {};
    }
    Room *room = m_rooms.at(index.row());
    switch (role) {
    case DisplayNameRole:
        return room->displayName();
    case AvatarRole:
        return room->avatarMediaId();
    case CanonicalAliasRole:
        return room->canonicalAlias();
    case TopicRole:
        return room->topic();
    case IdRole:
        return room->id();
    case AvatarImageRole:
        return room->avatar(48);
    case CategoryRole:
        if (room->joinState() == JoinState::Invite) {
            return InvitedRoom;
        }
        if (room->isFavourite()) {
            return FavoriteRoom;
        }
        if (room->isLowPriority()) {
            return LowPriorityRoom;
        }
        if (room->isDirectChat()) {
            return DirectChatRoom;
        }
        if (const RoomCreateEvent *creationEvent = room->creation(); creationEvent) {
            const auto contentJson = creationEvent->contentJson();
            if (contentJson.value(Quotient::TypeKey) == QLatin1StringView("m.space")) {
                return Space;
            }
        }
        return RegularRoom;
    }
    return {};
}

void MatrixRoomsModel::refresh(Room *room, const QList<int> &roles)
{
    const auto it = std::find(m_rooms.begin(), m_rooms.end(), room);
    if (it == m_rooms.end()) {
        return;
    }
    const auto idx = index(it - m_rooms.begin());
    Q_EMIT dataChanged(idx, idx, roles);
}

QHash<int, QByteArray> MatrixRoomsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DisplayNameRole] = "displayName";
    roles[AvatarRole] = "avatar";
    roles[CanonicalAliasRole] = "canonicalAlias";
    roles[TopicRole] = "topic";
    roles[IdRole] = "id";
    roles[AvatarImageRole] = "avatarImage";
    roles[CategoryRole] = "category";
    return roles;
}

#include "moc_matrixroomsmodel.cpp"
