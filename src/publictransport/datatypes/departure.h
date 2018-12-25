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

#ifndef KPUBLICTRANSPORT_DEPARTURE_H
#define KPUBLICTRANSPORT_DEPARTURE_H

#include "datatypes.h"
#include "line.h"
#include "location.h"

class QDateTime;

namespace KPublicTransport {

class DeparturePrivate;

/** Information about an arrival and/or departure of a vehicle at a stop area. */
class Departure
{
    KPUBLICTRANSPORT_GADGET(Departure)

    /** Planned arrival time. */
    Q_PROPERTY(QDateTime scheduledArrivalTime READ scheduledArrivalTime WRITE setScheduledArrivalTime)
    /** Actual arrival time, if available.
     *  Set to invalid to indicate real-time data is not available.
     */
    Q_PROPERTY(QDateTime expectedArrivalTime READ expectedArrivalTime WRITE setExpectedArrivalTime)
    /** @c true if this has real-time data. */
    Q_PROPERTY(bool hasExpectedArrivalTime READ hasExpectedArrivalTime STORED false)
    /** Difference to schedule in minutes. */
    Q_PROPERTY(int arrivalDelay READ arrivalDelay STORED false)

    /** Planned departure time. */
    Q_PROPERTY(QDateTime scheduledDepartureTime READ scheduledDepartureTime WRITE setScheduledDepartureTime)
    /** Actual departure time, if available.
     *  Set to invalid to indicate real-time data is not available.
     */
    Q_PROPERTY(QDateTime expectedDepartureTime READ expectedDepartureTime WRITE setExpectedDepartureTime)
    /** @c true if this has real-time data. */
    Q_PROPERTY(bool hasExpectedDepartureTime READ hasExpectedDepartureTime STORED false)
    /** Difference to schedule in minutes. */
    Q_PROPERTY(int departureDelay READ departureDelay STORED false)

    /** Planned departure platform. */
    Q_PROPERTY(QString scheduledPlatform READ scheduledPlatform WRITE setScheduledPlatform)
    /** Actual departure platform, in case real-time information are available. */
    Q_PROPERTY(QString expectedPlatform READ expectedPlatform WRITE setExpectedPlatform)
    /** @c true if real-time platform information are available. */
    Q_PROPERTY(bool hasExpectedPlatform READ hasExpectedPlatform STORED false)

    /** The departing route. */
    Q_PROPERTY(KPublicTransport::Route route READ route WRITE setRoute)

    /** The stop point of this departure. */
    Q_PROPERTY(KPublicTransport::Location stopPoint READ stopPoint WRITE setStopPoint)

public:
    QDateTime scheduledArrivalTime() const;
    void setScheduledArrivalTime(const QDateTime &scheduledTime);
    QDateTime expectedArrivalTime() const;
    void setExpectedArrivalTime(const QDateTime &expectedTime);
    bool hasExpectedArrivalTime() const;
    int arrivalDelay() const;

    QDateTime scheduledDepartureTime() const;
    void setScheduledDepartureTime(const QDateTime &scheduledTime);
    QDateTime expectedDepartureTime() const;
    void setExpectedDepartureTime(const QDateTime &expectedTime);
    bool hasExpectedDepartureTime() const;
    int departureDelay() const;

    QString scheduledPlatform() const;
    void setScheduledPlatform(const QString &platform);
    QString expectedPlatform() const;
    void setExpectedPlatform(const QString &platform);
    bool hasExpectedPlatform() const;

    Route route() const;
    void setRoute(const Route &route);
    Location stopPoint() const;
    void setStopPoint(const Location &stopPoint);

    /** Checks if to instances refer to the same departure (which does not necessarily mean they are exactly equal). */
    static bool isSame(const Departure &lhs, const Departure &rhs);

    /** Merge two departure instances.
     *  This assumes isSame(lhs, rhs) and tries to preserve the most detailed information.
     */
    static Departure merge(const Departure &lhs, const Departure &rhs);

    /** Serializes one object to JSON. */
    static QJsonObject toJson(const Departure &dep);
    /** Deserialize an object from JSON. */
    static Departure fromJson(const QJsonObject &obj);
};

}

Q_DECLARE_METATYPE(KPublicTransport::Departure)

#endif // KPUBLICTRANSPORT_DEPARTURE_H
