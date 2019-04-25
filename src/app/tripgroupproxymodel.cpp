/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "tripgroupproxymodel.h"
#include "timelinemodel.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"

#include <QDebug>
#include <QSettings>

TripGroupProxyModel::TripGroupProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    QSettings settings;
    settings.beginGroup(QLatin1String("TripGroupProxyState"));
    for (const auto &key : settings.childKeys()) {
        m_collapsed[key] = settings.value(key).toBool();
    }
}

TripGroupProxyModel::~TripGroupProxyModel() = default;

void TripGroupProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    m_sourceModel = qobject_cast<TimelineModel*>(sourceModel);
    Q_ASSERT(m_sourceModel);
    connect(m_sourceModel, &TimelineModel::todayRowChanged, this, &TripGroupProxyModel::todayRowChanged);
    connect(m_sourceModel, &TimelineModel::todayRowChanged, this, &TripGroupProxyModel::invalidateFilter);
    QSortFilterProxyModel::setSourceModel(m_sourceModel);
}

QVariant TripGroupProxyModel::data(const QModelIndex &index, int role) const
{
    if (role == TimelineModel::ElementRangeRole) {
        const auto srcIdx = mapToSource(index);
        const auto elementType = srcIdx.data(TimelineModel::ElementTypeRole).toInt();
        const auto rangeType = srcIdx.data(TimelineModel::ElementRangeRole).toInt();
        if (elementType == TimelineModel::TripGroup && rangeType == TimelineModel::RangeBegin) {
            const auto groupId = srcIdx.data(TimelineModel::TripGroupIdRole).toString();
            if (isCollapsed(groupId)) {
                return TimelineModel::SelfContained;
            }
        }
    }
    return QSortFilterProxyModel::data(index, role);
}

bool TripGroupProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    // ### we can probably do this more efficiently!
    for (int i = source_row; i < m_sourceModel->rowCount(); ++i) {
        const auto srcIdx = m_sourceModel->index(i, 0);
        const auto elementType = srcIdx.data(TimelineModel::ElementTypeRole).toInt();
        const auto rangeType = srcIdx.data(TimelineModel::ElementRangeRole).toInt();
        if (elementType == TimelineModel::TripGroup && rangeType == TimelineModel::RangeBegin) {
            // either this is our group's start, or a group after us
            return true;
        }
        if (elementType != TimelineModel::TripGroup) {
            continue;
        }
        const auto groupId = srcIdx.data(TimelineModel::TripGroupIdRole).toString();
        return !isCollapsed(groupId);
    }

    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

int TripGroupProxyModel::todayRow()
{
    const auto srcIdx = m_sourceModel->index(m_sourceModel->todayRow(), 0);
    return mapFromSource(srcIdx).row();
}

void TripGroupProxyModel::collapse(const QString &groupId)
{
    m_collapsed[groupId] = true;
    invalidateFilter();

    const auto startSrcIdx = m_sourceModel->match(m_sourceModel->index(0, 0), TimelineModel::TripGroupIdRole, groupId, 1, Qt::MatchExactly).at(0);
    Q_ASSERT(startSrcIdx.isValid());
    const auto startIdx = mapFromSource(startSrcIdx);
    emit dataChanged(startIdx, startIdx); // collapse/expand state changed

    QSettings settings;
    settings.beginGroup(QLatin1String("TripGroupProxyState"));
    settings.setValue(groupId, true);
}

void TripGroupProxyModel::expand(const QString &groupId)
{
    m_collapsed[groupId] = false;
    invalidateFilter();

    const auto startSrcIdx = m_sourceModel->match(m_sourceModel->index(0, 0), TimelineModel::TripGroupIdRole, groupId, 1, Qt::MatchExactly).at(0);
    Q_ASSERT(startSrcIdx.isValid());
    const auto startIdx = mapFromSource(startSrcIdx);
    emit dataChanged(startIdx, startIdx); // collapse/expand state changed

    QSettings settings;
    settings.beginGroup(QLatin1String("TripGroupProxyState"));
    settings.setValue(groupId, false);
}

bool TripGroupProxyModel::isCollapsed(const QString &groupId) const
{
    const auto g = m_sourceModel->tripGroupManager()->tripGroup(groupId);
    if (g.beginDateTime() <= m_sourceModel->now() && m_sourceModel->now() <= g.endDateTime()) {
        return false; // the current trip must always be expanded
    }

    const auto it = m_collapsed.constFind(groupId);
    if (it != m_collapsed.constEnd()) {
        return it.value();
    }

    // by default collapse past trips
    return g.endDateTime() < m_sourceModel->now();
}
