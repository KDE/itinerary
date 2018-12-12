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

/** Information about a departure of a vehicle at a stop area. */
class Departure
{
    KPUBLICTRANSPORT_GADGET(Departure)
    /** Planned departure time. */
    Q_PROPERTY(QDateTime scheduledTime READ scheduledTime WRITE setScheduledTime)
    /** Actual departure time, if available.
     *  Set to invalid to indicate real-time data is not available.
     */
    Q_PROPERTY(QDateTime actualTime READ actualTime WRITE setActualTime)
    /** @c true if this has real-time data. */
    Q_PROPERTY(bool hasRealTime READ hasRealTime STORED false)

    /** The departing route. */
    Q_PROPERTY(KPublicTransport::Route route READ route WRITE setRoute)

    /** The stop point of this departure. */
    Q_PROPERTY(KPublicTransport::Location stopPoint READ stopPoint WRITE setStopPoint)

public:
    QDateTime scheduledTime() const;
    void setScheduledTime(const QDateTime &scheduledTime);
    QDateTime actualTime() const;
    void setActualTime(const QDateTime &actualTime);

    bool hasRealTime() const;

    Route route() const;
    void setRoute(const Route &route);
    Location stopPoint() const;
    void setStopPoint(const Location &stopPoint);
};

}

Q_DECLARE_METATYPE(KPublicTransport::Departure)

#endif // KPUBLICTRANSPORT_DEPARTURE_H
