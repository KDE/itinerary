// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "matrixroomssortproxymodel.h"
#include "matrixroomsmodel.h"

MatrixRoomsSortProxyModel::MatrixRoomsSortProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortRole(MatrixRoomsModel::DisplayNameRole);
    setSortLocaleAware(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setDynamicSortFilter(true);
}

MatrixRoomsSortProxyModel::~MatrixRoomsSortProxyModel() = default;

void MatrixRoomsSortProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);
    sort(0);
}

bool MatrixRoomsSortProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    const auto lhsCategory = source_left.data(MatrixRoomsModel::CategoryRole).toInt();
    const auto rhsCategory = source_right.data(MatrixRoomsModel::CategoryRole).toInt();
    if (lhsCategory == rhsCategory) {
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }

    return lhsCategory < rhsCategory;
}

bool MatrixRoomsSortProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto category = sourceModel()->data(sourceModel()->index(source_row, 0, source_parent), MatrixRoomsModel::CategoryRole).toInt();
    if (category == MatrixRoomsModel::Space) {
        return false;
    }
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

#include "moc_matrixroomssortproxymodel.cpp"
