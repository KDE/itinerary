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

#include "departure.h"
#include "datatypes_p.h"

#include <QDateTime>

using namespace KPublicTransport;

namespace KPublicTransport {
class DeparturePrivate : public QSharedData {
public:
    QDateTime scheduledTime;
    QDateTime actualTime;
    Route route;
    Location stopPoint;
};
}

KPUBLICTRANSPORT_MAKE_GADGET(Departure)

QDateTime Departure::scheduledTime() const
{
    return d->scheduledTime;
}

void Departure::setScheduledTime(const QDateTime &scheduledTime)
{
    d.detach();
    d->scheduledTime = scheduledTime;
}

QDateTime Departure::actualTime() const
{
    return d->actualTime;
}

void Departure::setActualTime(const QDateTime &actualTime)
{
    d.detach();
    d->actualTime = actualTime;
}

Route Departure::route() const
{
    return d->route;
}

void Departure::setRoute(const Route &route)
{
    d.detach();
    d->route = route;
}

Location Departure::stopPoint() const
{
    return d->stopPoint;
}

void Departure::setStopPoint(const Location &stopPoint)
{
    d.detach();
    d->stopPoint = stopPoint;
}
