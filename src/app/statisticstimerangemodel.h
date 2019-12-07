/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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
