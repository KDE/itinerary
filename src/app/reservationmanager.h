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

#ifndef RESERVATIONMANAGER_H
#define RESERVATIONMANAGER_H

#include <QHash>
#include <QObject>
#include <QVariant>

class PkPassManager;

class QUrl;

/** Manages JSON-LD reservation data. */
class ReservationManager : public QObject
{
    Q_OBJECT
public:
    ReservationManager(QObject *parent = nullptr);
    ~ReservationManager();

    void setPkPassManager(PkPassManager *mgr);

    bool hasReservation(const QString &id) const;
    QVector<QString> reservations() const;
    Q_INVOKABLE QVariant reservation(const QString &id) const;

    Q_INVOKABLE void addReservation(const QVariant &res);
    Q_INVOKABLE void updateReservation(const QString &resId, const QVariant &res);
    Q_INVOKABLE void removeReservation(const QString &id);
    Q_INVOKABLE void removeReservations(const QStringList &ids);

    void importReservation(const QByteArray &data);
    void importReservations(const QVector<QVariant> &resData);

signals:
    void reservationAdded(const QString &id);
    void reservationUpdated(const QString &id);
    void reservationRemoved(const QString &id);

private:
    static QString basePath();

    void passAdded(const QString &passId);
    void passUpdated(const QString &passId);
    void passRemoved(const QString &passId);

    mutable QHash<QString, QVariant> m_reservations;
    PkPassManager *m_passMgr = nullptr;
};

#endif // RESERVATIONMANAGER_H
