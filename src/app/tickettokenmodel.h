/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    Q_PROPERTY(int initialIndex READ initialIndex NOTIFY initialIndexChanged)

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
    Q_INVOKABLE QString reservationIdAt(int row) const;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int initialIndex() const;

Q_SIGNALS:
    void initialIndexChanged();

private:
    ReservationManager *m_resMgr = nullptr;
    QStringList m_pendingResIds;
    QStringList m_resIds;
    QStringList m_personNames;
};

#endif // TICKETTOKENMODEL_H
