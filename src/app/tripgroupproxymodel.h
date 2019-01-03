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

#ifndef TRIPGROUPPROXYMODEL_H
#define TRIPGROUPPROXYMODEL_H

#include <QSet>
#include <QSortFilterProxyModel>

class TimelineModel;

/** Proxy model for expanding/collapsing trip groups. */
class TripGroupProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int todayRow READ todayRow NOTIFY todayRowChanged)
public:
    explicit TripGroupProxyModel(QObject *parent = nullptr);
    ~TripGroupProxyModel();

    void setSourceModel(QAbstractItemModel *sourceModel) override;
    QVariant data(const QModelIndex & index, int role) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const override;

    Q_INVOKABLE void collapse(const QString &groupId);
    Q_INVOKABLE void expand(const QString &groupId);

    // "override" from TimelineModel
    int todayRow();

Q_SIGNALS:
    void todayRowChanged();

private:
    bool isCollapsed(const QString &groupId) const;

    TimelineModel *m_sourceModel = nullptr;
    QHash<QString, bool> m_collapsed;
};

#endif // TRIPGROUPPROXYMODEL_H
