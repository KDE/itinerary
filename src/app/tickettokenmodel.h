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

#ifndef TICKETTOKENMODEL_H
#define TICKETTOKENMODEL_H

#include <QAbstractListModel>

class ReservationManager;

/** Filtered model of all reservations with a valid ticket token
 *  for display in the details page.
 */
class TicketTokenModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QObject* reservationManager READ reservationManager WRITE setReservationManager)
    Q_PROPERTY(QStringList reservationIds READ reservationIds WRITE setReservationIds)
public:
    enum Roles {
        ReservationRole = Qt::UserRole
    };

    explicit TicketTokenModel(QObject *parent = nullptr);
    ~TicketTokenModel() override;

    QObject* reservationManager() const;
    void setReservationManager(QObject *mgr);
    QStringList reservationIds() const;
    void setReservationIds(const QStringList &resIds);

    Q_INVOKABLE QVariant reservationAt(int row) const;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    ReservationManager *m_resMgr = nullptr;
    QStringList m_pendingResIds;
    QStringList m_resIds;
};

#endif // TICKETTOKENMODEL_H
