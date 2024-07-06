// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "tripgroupmodel.h"

#include "logging.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"

TripGroupModel::TripGroupModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_updateTimer.setTimerType(Qt::VeryCoarseTimer);
    m_updateTimer.setSingleShot(true);
    connect(&m_updateTimer, &QTimer::timeout, this, [this]() {
        if (m_tripGroups.empty()) {
            return;
        }
        Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0));
        scheduleUpdate();
    });
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
    m_tripGroups = m_tripGroupManager->tripGroups();
    std::sort(m_tripGroups.begin(), m_tripGroups.end(), [this](const auto &lhs, const auto &rhs) {
        return tripGroupLessThan(lhs, rhs);
    });

    connect(m_tripGroupManager, &TripGroupManager::tripGroupAdded, this, &TripGroupModel::tripGroupAdded);
    connect(m_tripGroupManager, &TripGroupManager::tripGroupChanged, this, &TripGroupModel::tripGroupChanged);
    connect(m_tripGroupManager, &TripGroupManager::tripGroupRemoved, this, &TripGroupModel::tripGroupRemoved);

    endResetModel();
    scheduleUpdate();

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
        // NOTE: when adjusting this adjust scheduleUpdate accordingly!
        if (today() > tripGroup.endDateTime().date()) {
            return Position::Past;
        }

        if (now() < tripGroup.beginDateTime().date().startOfDay()) {
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
    const auto it = std::lower_bound(m_tripGroups.begin(), m_tripGroups.end(), tgId, [this](const auto &lhs, const auto &rhs) {
        return tripGroupLessThan(lhs, rhs);
    });
    const auto row = (int)std::distance(m_tripGroups.begin(), it);
    beginInsertRows({}, row, row);
    m_tripGroups.insert(it, tgId);
    endInsertRows();
    scheduleUpdate();
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
        scheduleUpdate();
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
    scheduleUpdate();
}

bool TripGroupModel::tripGroupLessThan(const QString &lhs, const QString &rhs) const
{
    return m_tripGroupManager->tripGroup(lhs).beginDateTime() > m_tripGroupManager->tripGroup(rhs).beginDateTime();
}

bool TripGroupModel::tripGroupLessThan(const QString &lhs, const QDateTime &rhs) const
{
    return m_tripGroupManager->tripGroup(lhs).beginDateTime() > rhs;
}

void TripGroupModel::setCurrentDateTime(const QDateTime &dt)
{
    m_unitTestTime = dt;
}

void TripGroupModel::scheduleUpdate()
{
    const auto currentDt = now();

    // find first current or future element
    auto it = std::lower_bound(m_tripGroups.begin(), m_tripGroups.end(), currentDt, [this](const auto &lhs, const auto &rhs) {
        return tripGroupLessThan(lhs, rhs);
    });
    while (it != m_tripGroups.begin() && (it == m_tripGroups.end() || m_tripGroupManager->tripGroup(*it).endDateTime() > currentDt)) {
        --it;
    }

    QDateTime dt;
    for (; it != m_tripGroups.end(); ++it) {
        const auto tg = m_tripGroupManager->tripGroup(*it);
        if (dt.isValid() && tg.beginDateTime().date().startOfDay() > dt) {
            break;
        }
        if (const auto beginDt = tg.beginDateTime().date().startOfDay(); beginDt > currentDt) {
            dt = dt.isValid() ? std::min(dt, beginDt) : beginDt;
        }
        if (const auto endDt = tg.endDateTime().date().endOfDay(); endDt > currentDt) {
            dt = dt.isValid() ? std::min(dt, endDt) : endDt;
        }
    }

    if (!dt.isValid()) {
        return;
    }

    const auto timeDelta = std::chrono::seconds(std::max(60ll, currentDt.secsTo(dt)));
    if (timeDelta.count() * 1000 > std::numeric_limits<int>::max()) {
        qCDebug(Log) << "not scheduling trip group model update, would overflow QTimer" << dt;
        m_updateTimer.start(std::numeric_limits<int>::max());
    } else {
        qCDebug(Log) << "scheduling next trip group update for" << dt;
        m_updateTimer.start(timeDelta);
    }
}

QDateTime TripGroupModel::now() const
{
    if (Q_UNLIKELY(m_unitTestTime.isValid())) {
        return m_unitTestTime;
    }
    return QDateTime::currentDateTime();
}

QDate TripGroupModel::today() const
{
    if (Q_UNLIKELY(m_unitTestTime.isValid())) {
        return m_unitTestTime.date();
    }
    return QDate::currentDate();
}

#include "moc_tripgroupmodel.cpp"
