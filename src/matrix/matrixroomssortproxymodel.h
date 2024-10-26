// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef MATRIXROOMSSORTPROXYMODEL_H
#define MATRIXROOMSSORTPROXYMODEL_H

#include <QSortFilterProxyModel>

/** Sorting proxy for matrix rooms. */
// TODO this probably should have configurable sorting modes (activity, alphabetically, etc)
class MatrixRoomsSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit MatrixRoomsSortProxyModel(QObject *parent = nullptr);
    ~MatrixRoomsSortProxyModel();

    void setSourceModel(QAbstractItemModel *sourceModel) override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};

#endif // MATRIXROOMSSORTPROXYMODEL_H
