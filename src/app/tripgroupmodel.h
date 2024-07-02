// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QtQml>

#include "tripgroupmanager.h"
#include "tripgroup.h"

class TripGroupModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(TripGroupManager *tripGroupManager READ tripGroupManager WRITE setTripGroupManager NOTIFY tripGroupManagerChanged)

public:
    enum ExtraRoles {
        BeginRole = Qt::UserRole + 1,
        EndRole,
        PositionRole,
        TripGroupRole,
    };

    enum Position {
        Past,
        Current,
        Future,
    };
    Q_ENUM(Position);

    explicit TripGroupModel(QObject *parent = nullptr);

    [[nodiscard]]
    TripGroupManager *tripGroupManager() const;
    void setTripGroupManager(TripGroupManager *tripGroupManager);

    [[nodiscard]]
    QVariant data(const QModelIndex &index, int role) const override;

    [[nodiscard]]
    QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]]
    int rowCount(const QModelIndex& parent) const override;

Q_SIGNALS:
    void tripGroupManagerChanged();

private:
    TripGroupManager *m_tripGroupManager = nullptr;
    std::vector<TripGroup> m_tripGroups;
};
