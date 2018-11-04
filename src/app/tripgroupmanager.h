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

#ifndef TRIPGROUPMANAGER_H
#define TRIPGROUPMANAGER_H

#include <QObject>
#include <QHash>

class ReservationManager;
class TripGroup;
class TripGroupTest;

/** Trip group computation and persistence. */
class TripGroupManager : public QObject
{
    Q_OBJECT
public:
    explicit TripGroupManager(QObject *parent = nullptr);
    ~TripGroupManager();

    void setReservationManager(ReservationManager *resMgr);

    QVector<QString> tripGroups() const;
    TripGroup tripGroup(const QString &id) const;
    TripGroup tripGroupForReservation(const QString &resId) const;

signals:
    void tripGroupAdded(const QString &id);
    void tripGroupChanged(const QString &id);
    void tripGroupRemoved(const QString &id);

private:
    friend class TripGroupTest;
    friend class TimelineModelTest;

    static QString basePath();
    void load();
    static void clear(); // for testing only!

    void reservationAdded(const QString &resId);
    void reservationChanged(const QString &resId);
    void reservationRemoved(const QString &resId);

    void scanAll();
    void scanOne(const std::vector<QString>::const_iterator &beginIt);
    QString guessName(const TripGroup &g) const;

    ReservationManager *m_resMgr = nullptr;
    QHash<QString, TripGroup> m_tripGroups;
    QHash<QString, QString> m_reservationToGroupMap;

    std::vector<QString> m_reservations;

    struct ReservationNumberSearch {
        int type;
        QString resNum;
    };
    std::vector<ReservationNumberSearch> m_resNumSearch;
};

#endif // TRIPGROUPMANAGER_H
