/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    ~TripGroupProxyModel() override;

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
