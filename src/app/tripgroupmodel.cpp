// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "tripgroupmodel.h"

#include "logging.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"

using namespace std::placeholders;

TripGroupModel::TripGroupModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

TripGroupModel::~TripGroupModel() = default;

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
    const auto tgIds = m_tripGroupManager->tripGroups();
    m_tripGroups.reserve(tgIds.size());
    std::copy(tgIds.begin(), tgIds.end(), std::back_inserter(m_tripGroups));

    connect(m_tripGroupManager, &TripGroupManager::tripGroupAdded, this, &TripGroupModel::tripGroupAdded);
    connect(m_tripGroupManager, &TripGroupManager::tripGroupChanged, this, &TripGroupModel::tripGroupChanged);
    connect(m_tripGroupManager, &TripGroupManager::tripGroupRemoved, this, &TripGroupModel::tripGroupRemoved);

    endResetModel();

    Q_EMIT tripGroupManagerChanged();
}

QVariant TripGroupModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));

    const auto &tgId = m_tripGroups[index.row()];
    const auto tripGroup = m_tripGroupManager->tripGroup(tgId);

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
    case TripGroupIdRole:
        return tgId;
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
        { TripGroupIdRole, "tripGroupId" },
    };
}

int TripGroupModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return int(m_tripGroups.size());
}

void TripGroupModel::tripGroupAdded(const QString &tgId)
{
    const auto it = std::lower_bound(m_tripGroups.begin(), m_tripGroups.end(), tgId, std::bind(&TripGroupModel::tripGroupLessThan, this, _1, _2));
    const auto row = (int)std::distance(m_tripGroups.begin(), it);
    beginInsertRows({}, row, row);
    m_tripGroups.insert(it, tgId);
    endInsertRows();
}

void TripGroupModel::tripGroupChanged(const QString &tgId)
{
    const auto it = std::find(m_tripGroups.begin(), m_tripGroups.end(), tgId);
    if (it == m_tripGroups.end()) {
        qCCritical(Log) << "got change for an unknown trip group";
        tripGroupAdded(tgId);
        return;
    }

    // check if sort order changed
    bool order = true;
    if (it != m_tripGroups.begin()) {
        order &= tripGroupLessThan(*std::prev(it), tgId);
    }
    if (std::next(it) != m_tripGroups.end()) {
        order &= tripGroupLessThan(tgId, *std::next(it));
    }

    if (order) {
        const auto row = (int)std::distance(m_tripGroups.begin(), it);
        const auto idx = index(row, 0);
        Q_EMIT dataChanged(idx, idx);
    } else {
        tripGroupRemoved(tgId);
        tripGroupAdded(tgId);
    }
}

void TripGroupModel::tripGroupRemoved(const QString &tgId)
{
    const auto it = std::find(m_tripGroups.begin(), m_tripGroups.end(), tgId);
    if (it == m_tripGroups.end()) {
        return;
    }
    const auto row = (int)std::distance(m_tripGroups.begin(), it);
    beginRemoveRows({}, row, row);
    m_tripGroups.erase(it);
    endRemoveRows();
}

bool TripGroupModel::tripGroupLessThan(const QString &lhs, const QString &rhs) const
{
    return m_tripGroupManager->tripGroup(lhs).beginDateTime() < m_tripGroupManager->tripGroup(rhs).beginDateTime();
}

#include "moc_tripgroupmodel.cpp"
