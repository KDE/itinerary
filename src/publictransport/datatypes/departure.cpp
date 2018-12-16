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
    QDateTime expectedTime;
    QString scheduledPlatform;
    QString expectedPlatform;
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

QDateTime Departure::expectedTime() const
{
    return d->expectedTime;
}

void Departure::setExpectedTime(const QDateTime &expectedTime)
{
    d.detach();
    d->expectedTime = expectedTime;
}

bool Departure::hasExpectedTime() const
{
    return d->expectedTime.isValid();
}

QString Departure::scheduledPlatform() const
{
    return d->scheduledPlatform;
}

void Departure::setScheduledPlatform(const QString &platform)
{
    d.detach();
    d->scheduledPlatform = platform;
}

QString Departure::expectedPlatform() const
{
    return d->expectedPlatform;
}

void Departure::setExpectedPlatform(const QString &platform)
{
    d.detach();
    d->expectedPlatform = platform;
}

bool Departure::hasExpectedPlatform() const
{
    return !d->expectedPlatform.isEmpty();
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

#include "moc_departure.cpp"
