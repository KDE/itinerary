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
#include "json.h"

#include <QDateTime>

using namespace KPublicTransport;

namespace KPublicTransport {
class DeparturePrivate : public QSharedData {
public:
    QDateTime scheduledArrivalTime;
    QDateTime expectedArrivalTime;
    QDateTime scheduledDepartureTime;
    QDateTime expectedDepartureTime;
    QString scheduledPlatform;
    QString expectedPlatform;
    Route route;
    Location stopPoint;
};
}

KPUBLICTRANSPORT_MAKE_GADGET(Departure)

QDateTime Departure::scheduledArrivalTime() const
{
    return d->scheduledArrivalTime;
}

void Departure::setScheduledArrivalTime(const QDateTime &scheduledTime)
{
    d.detach();
    d->scheduledArrivalTime = scheduledTime;
}

QDateTime Departure::expectedArrivalTime() const
{
    return d->expectedArrivalTime;
}

void Departure::setExpectedArrivalTime(const QDateTime &expectedTime)
{
    d.detach();
    d->expectedArrivalTime = expectedTime;
}

bool Departure::hasExpectedArrivalTime() const
{
    return d->expectedArrivalTime.isValid();
}

int Departure::arrivalDelay() const
{
    if (hasExpectedArrivalTime()) {
        return d->scheduledArrivalTime.secsTo(d->expectedArrivalTime) / 60;
    }
    return 0;
}

QDateTime Departure::scheduledDepartureTime() const
{
    return d->scheduledDepartureTime;
}

void Departure::setScheduledDepartureTime(const QDateTime &scheduledTime)
{
    d.detach();
    d->scheduledDepartureTime = scheduledTime;
}

QDateTime Departure::expectedDepartureTime() const
{
    return d->expectedDepartureTime;
}

void Departure::setExpectedDepartureTime(const QDateTime &expectedTime)
{
    d.detach();
    d->expectedDepartureTime = expectedTime;
}

bool Departure::hasExpectedDepartureTime() const
{
    return d->expectedDepartureTime.isValid();
}

int Departure::departureDelay() const
{
    if (hasExpectedDepartureTime()) {
        return d->scheduledDepartureTime.secsTo(d->expectedDepartureTime) / 60;
    }
    return 0;
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

bool Departure::platformChanged() const
{
    return hasExpectedPlatform() && d->expectedPlatform != d->scheduledPlatform;
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

bool Departure::isSame(const Departure &lhs, const Departure &rhs)
{
    if ((lhs.scheduledDepartureTime().isValid() && lhs.scheduledDepartureTime() != rhs.scheduledDepartureTime()) ||
        (lhs.scheduledArrivalTime().isValid() && lhs.scheduledArrivalTime() != rhs.scheduledArrivalTime())) {
        return false;
    }

    return Route::isSame(lhs.route(), rhs.route());
}

Departure Departure::merge(const Departure &lhs, const Departure &rhs)
{
    auto dep = lhs;
    if (!dep.scheduledDepartureTime().isValid() && rhs.scheduledDepartureTime().isValid()) {
        dep.setScheduledDepartureTime(rhs.scheduledDepartureTime());
    }
    if (!dep.hasExpectedDepartureTime() && rhs.hasExpectedDepartureTime()) {
        dep.setExpectedDepartureTime(rhs.expectedDepartureTime());
    }
    if (!dep.scheduledArrivalTime().isValid() && rhs.scheduledArrivalTime().isValid()) {
        dep.setScheduledArrivalTime(rhs.scheduledArrivalTime());
    }
    if (!dep.hasExpectedArrivalTime() && rhs.hasExpectedArrivalTime()) {
        dep.setExpectedArrivalTime(rhs.expectedArrivalTime());
    }
    if (dep.scheduledPlatform().isEmpty() && !rhs.scheduledPlatform().isEmpty()) {
        dep.setScheduledPlatform(rhs.scheduledPlatform());
    }
    if (!dep.hasExpectedPlatform() && rhs.hasExpectedPlatform()) {
        dep.setExpectedPlatform(rhs.expectedPlatform());
    }

    dep.setRoute(Route::merge(lhs.route(), rhs.route()));
    return dep;
}

QJsonObject Departure::toJson(const Departure &dep)
{
    auto obj = Json::toJson(dep);
    obj.insert(QLatin1String("route"), Route::toJson(dep.route()));
    obj.insert(QLatin1String("stopPoint"), Location::toJson(dep.stopPoint()));
    return obj;
}

Departure Departure::fromJson(const QJsonObject &obj)
{
    auto dep = Json::fromJson<Departure>(obj);
    dep.setRoute(Route::fromJson(obj.value(QLatin1String("route")).toObject()));
    dep.setStopPoint(Location::fromJson(obj.value(QLatin1String("stopPoint")).toObject()));
    return dep;
}

#include "moc_departure.cpp"
