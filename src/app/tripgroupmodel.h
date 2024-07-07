// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QTimer>
#include <QtQml>

class TripGroupManager;

/** List of all trip groups in reverse chronological order. */
class TripGroupModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(TripGroupManager *tripGroupManager READ tripGroupManager WRITE setTripGroupManager NOTIFY tripGroupManagerChanged)

public:
    enum ExtraRoles {
        BeginRole = Qt::UserRole + 1,
        EndRole,
        PositionRole,
        TripGroupRole,
        TripGroupIdRole,
    };

    enum Position {
        Past,
        Current,
        Future,
    };
    Q_ENUM(Position);

    explicit TripGroupModel(QObject *parent = nullptr);
    ~TripGroupModel();

    [[nodiscard]]
    TripGroupManager *tripGroupManager() const;
    void setTripGroupManager(TripGroupManager *tripGroupManager);

    // QAbstractListModel interface
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;

    // adjacecy queries
    /** Trips next to @p tripGroupId which are suitable candidates for merging. */
    Q_INVOKABLE [[nodiscard]] QStringList adjacentTripGroups(const QString &tripGroupId) const;
    /** Trips adjacent to the given time frame (ie. possible candidates for merging). */
    Q_INVOKABLE [[nodiscard]] QStringList adjacentTripGroups(const QDateTime &from, const QDateTime &to) const;
    /** Trips intersecting with the given time frame. */
    Q_INVOKABLE [[nodiscard]] QStringList intersectingTripGroups(const QDateTime &from, const QDateTime &to) const;

    // for unit testing
    void setCurrentDateTime(const QDateTime &dt);
    [[nodiscard]] QDateTime now() const;
    [[nodiscard]] QDate today() const;

Q_SIGNALS:
    void tripGroupManagerChanged();

private:
    void tripGroupAdded(const QString &tgId);
    void tripGroupChanged(const QString &tgId);
    void tripGroupRemoved(const QString &tgId);

    [[nodiscard]] bool tripGroupLessThan(const QString &lhs, const QString &rhs) const;
    [[nodiscard]] bool tripGroupLessThan(const QString &lhs, const QDateTime &rhs) const;

    void scheduleUpdate();

    TripGroupManager *m_tripGroupManager = nullptr;
    std::vector<QString> m_tripGroups;
    QTimer m_updateTimer;
    QDateTime m_unitTestTime;
};
