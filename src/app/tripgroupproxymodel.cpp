/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    settings.beginGroup(QLatin1StringView("TripGroupProxyState"));
    for (const auto &key : settings.childKeys()) {
        m_collapsed[key] = settings.value(key).toBool();
    }
}

TripGroupProxyModel::~TripGroupProxyModel() = default;

void TripGroupProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    m_sourceModel = qobject_cast<TimelineModel *>(sourceModel);
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
        if (elementType == TimelineElement::TripGroup && rangeType == TimelineElement::RangeBegin) {
            const auto groupId = srcIdx.data(TimelineModel::TripGroupIdRole).toString();
            if (isCollapsed(groupId)) {
                return TimelineElement::SelfContained;
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
        if (elementType == TimelineElement::TripGroup && rangeType == TimelineElement::RangeBegin) {
            // either this is our group's start, or a group after us
            return true;
        }
        if (elementType != TimelineElement::TripGroup) {
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
    Q_EMIT dataChanged(startIdx, startIdx); // collapse/expand state changed

    QSettings settings;
    settings.beginGroup(QLatin1StringView("TripGroupProxyState"));
    settings.setValue(groupId, true);
}

void TripGroupProxyModel::expand(const QString &groupId)
{
    m_collapsed[groupId] = false;
    invalidateFilter();

    const auto startSrcIdx = m_sourceModel->match(m_sourceModel->index(0, 0), TimelineModel::TripGroupIdRole, groupId, 1, Qt::MatchExactly).at(0);
    Q_ASSERT(startSrcIdx.isValid());
    const auto startIdx = mapFromSource(startSrcIdx);
    Q_EMIT dataChanged(startIdx, startIdx); // collapse/expand state changed

    QSettings settings;
    settings.beginGroup(QLatin1StringView("TripGroupProxyState"));
    settings.setValue(groupId, false);
}

bool TripGroupProxyModel::isCollapsed(const QString &groupId) const
{
    const auto g = m_sourceModel->tripGroupManager()->tripGroup(groupId);
    const auto now = m_sourceModel->now();
    if (g.beginDateTime() <= now && now <= g.endDateTime()) {
        return false; // the current trip must always be expanded
    }

    const auto it = m_collapsed.constFind(groupId);
    if (it != m_collapsed.constEnd()) {
        return it.value();
    }

    // by default collapse past trips
    return g.endDateTime() < now;
}

#include "moc_tripgroupproxymodel.cpp"
