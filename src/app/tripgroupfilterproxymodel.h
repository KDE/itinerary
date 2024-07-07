// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef TRIPGROUPFILTERPROXYMODEL_H
#define TRIPGROUPFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

/** Filter proxy model to show only a specified set of trip groups.
 *  Used e.g. for selecting candidates to add to/merge with.
 */
class TripGroupFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList filteredGroupIds READ filteredGroupIds WRITE setFilteredGroupIds NOTIFY filteredGroupIdsChanged)
public:
    explicit TripGroupFilterProxyModel(QObject *parent = nullptr);
    ~TripGroupFilterProxyModel();

    [[nodiscard]] QStringList filteredGroupIds() const;
    void setFilteredGroupIds(const QStringList &groupIds);

Q_SIGNALS:
    void filteredGroupIdsChanged();

protected:
    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QStringList m_groupIds;
};

#endif
