/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef STATISTICSTIMERANGEMODEL_H
#define STATISTICSTIMERANGEMODEL_H

#include <QAbstractListModel>

class ReservationManager;

/** Selectable time ranges for the statistics page. */
class StatisticsTimeRangeModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(ReservationManager* reservationManager READ reservationManager WRITE setReservationManager NOTIFY setupChanged)

public:
    explicit StatisticsTimeRangeModel(QObject *parent = nullptr);
    ~StatisticsTimeRangeModel();

    enum {
        BeginRole = Qt::UserRole,
        EndRole
    };

    ReservationManager* reservationManager() const;
    void setReservationManager(ReservationManager *resMgr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void setupChanged();

private:
    ReservationManager *m_resMgr = nullptr;
    std::vector<int> m_years;
};

#endif // STATISTICSTIMERANGEMODEL_H
