// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <events/roomevent.h>

#include <QAbstractListModel>


namespace Quotient
{
    class Connection;
    class Room;
}

class MatrixRoomsModel : public QAbstractListModel
{
Q_OBJECT
    Q_PROPERTY(Quotient::Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

public:
    enum EventRoles {
        DisplayNameRole = Qt::DisplayRole,
        AvatarRole,
        CanonicalAliasRole,
        TopicRole,
        IdRole,
    };
    Q_ENUM(EventRoles)

    MatrixRoomsModel(QObject *parent = nullptr);

    [[nodiscard]] Quotient::Connection *connection() const
    {
        return m_connection;
    }
    void setConnection(Quotient::Connection *connection);
    void doResetModel();

    Q_INVOKABLE [[nodiscard]] Quotient::Room *roomAt(int row) const;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private Q_SLOTS:
    void doAddRoom(Quotient::Room *room);
    void updateRoom(Quotient::Room *room, Quotient::Room *prev);
    void deleteRoom(Quotient::Room *room);
    void refresh(Quotient::Room *room, const QVector<int> &roles = {});

private:
    Quotient::Connection *m_connection = nullptr;

    QList<Quotient::Room *> m_rooms;

    void connectRoomSignals(Quotient::Room *room);
Q_SIGNALS:
    void connectionChanged();
};
