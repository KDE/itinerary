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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RESERVATIONMANAGER_H
#define RESERVATIONMANAGER_H

#include <QHash>
#include <QObject>

/** Manages JSON-LD reservation data. */
class ReservationManager : public QObject
{
    Q_OBJECT
public:
    ReservationManager(QObject *parent = nullptr);
    ~ReservationManager();

    QVector<QString> reservations() const;
    QVariant reservation(const QString &id) const;

    void importReservation(const QString &filename);
    void removeReservation(const QString &id);

signals:
    void reservationAdded(const QString &id);
    void reservationUpdated(const QString &id);
    void reservationRemoved(const QString &id);

private:
    mutable QHash<QString, QVariant> m_reservations;
};

#endif // RESERVATIONMANAGER_H
