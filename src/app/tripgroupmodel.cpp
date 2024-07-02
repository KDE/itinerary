// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "tripgroupmodel.h"

TripGroupModel::TripGroupModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

TripGroupManager *TripGroupModel::tripGroupManager() const
{
    return m_tripGroupManager;
}

void TripGroupModel::setTripGroupManager(TripGroupManager *tripGroupManager)
{
    if (m_tripGroupManager == tripGroupManager) {
        return;
    }

    beginResetModel();
    m_tripGroupManager = tripGroupManager;
    endResetModel();

    Q_EMIT tripGroupManagerChanged();
}

QVariant TripGroupModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));

    const auto &tripGroup = m_tripGroupManager->tripGroup(m_tripGroupManager->tripGroups()[index.row()]);

    switch (role) {
    case Qt::DisplayRole:
        return tripGroup.name();
    case BeginRole:
        return tripGroup.beginDateTime();
    case EndRole:
        return tripGroup.endDateTime();
    case PositionRole:
        if (QDate::currentDate() > tripGroup.endDateTime().date()) {
            return Position::Past;
        }

        if (QDateTime::currentDateTime() > tripGroup.beginDateTime().date().startOfDay()) {
            return Position::Future;
        }
        return Position::Current;
    case TripGroupRole:
        return QVariant::fromValue(tripGroup);
    default:
        return {};
    }
}

QHash<int, QByteArray> TripGroupModel::roleNames() const
{
    return {
        { Qt::DisplayRole, "name" },
        { BeginRole, "begin" },
        { EndRole, "end" },
        { PositionRole, "position" },
        { TripGroupRole, "tripGroup" },
    };
}

int TripGroupModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_tripGroupManager) {
        return 0;
    }

    return int(m_tripGroupManager->tripGroups().size());
}
