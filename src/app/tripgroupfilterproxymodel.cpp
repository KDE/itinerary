// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "tripgroupfilterproxymodel.h"

#include "tripgroupmodel.h"

TripGroupFilterProxyModel::TripGroupFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

TripGroupFilterProxyModel::~TripGroupFilterProxyModel() = default;

QStringList TripGroupFilterProxyModel::filteredGroupIds() const
{
    return m_groupIds;
}

void TripGroupFilterProxyModel::setFilteredGroupIds(const QStringList &groupIds)
{
    if (m_groupIds == groupIds) {
        return;
    }

    m_groupIds = groupIds;
    std::sort(m_groupIds.begin(), m_groupIds.end());
    Q_EMIT filteredGroupIdsChanged();
    invalidateRowsFilter();
}

bool TripGroupFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto srcIdx = sourceModel()->index(source_row, 0, source_parent);
    const auto tgId = sourceModel()->data(srcIdx, TripGroupModel::TripGroupIdRole).toString();
    return std::binary_search(m_groupIds.begin(), m_groupIds.end(), tgId);
}

#include "moc_tripgroupfilterproxymodel.cpp"
