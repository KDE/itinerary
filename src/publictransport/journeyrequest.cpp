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

#include "journeyrequest.h"

#include <KPublicTransport/Location>

#include <QDateTime>
#include <QSharedData>

using namespace KPublicTransport;

namespace KPublicTransport {
class JourneyRequestPrivate : public QSharedData {
public:
    Location from;
    Location to;
    QDateTime dateTime;
    JourneyRequest::DateTimeMode dateTimeMode = JourneyRequest::Departure;
};
}

JourneyRequest::JourneyRequest() :
    d(new JourneyRequestPrivate)
{
}

JourneyRequest::JourneyRequest(const Location &from, const Location &to)
    : d(new JourneyRequestPrivate)
{
    d->from = from;
    d->to = to;
}

JourneyRequest::JourneyRequest(JourneyRequest&&) noexcept = default;
JourneyRequest::JourneyRequest(const JourneyRequest&) = default;
JourneyRequest::~JourneyRequest() = default;
JourneyRequest& JourneyRequest::operator=(const JourneyRequest&) = default;

Location JourneyRequest::from() const
{
    return d->from;
}

Location JourneyRequest::to() const
{
    return d->to;
}

QDateTime JourneyRequest::dateTime() const
{
    return d->dateTime;
}

JourneyRequest::DateTimeMode JourneyRequest::dateTimeMode() const
{
    return d->dateTimeMode;
}

void JourneyRequest::setDeparutreTime(const QDateTime &dt)
{
    d.detach();
    d->dateTime = dt;
    d->dateTimeMode = Departure;
}

void JourneyRequest::setArrivalTime(const QDateTime &dt)
{
    d.detach();
    d->dateTime = dt;
    d->dateTimeMode = Arrival;
}
