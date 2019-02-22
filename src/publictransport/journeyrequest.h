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

#ifndef KPUBLICTRANSPORT_JOURNEYREQUEST_H
#define KPUBLICTRANSPORT_JOURNEYREQUEST_H

#include "kpublictransport_export.h"

#include <QSharedDataPointer>

class QDateTime;

namespace KPublicTransport {

class JourneyRequestPrivate;
class Location;

/** Describes a journey search. */
class KPUBLICTRANSPORT_EXPORT JourneyRequest
{
public:
    JourneyRequest();
    JourneyRequest(const Location &from, const Location &to);
    JourneyRequest(JourneyRequest&&) noexcept;
    JourneyRequest(const JourneyRequest &);
    ~JourneyRequest();
    JourneyRequest& operator=(const JourneyRequest&);

    Location from() const;
    Location to() const;

    /** Date/time at which the journey should start/end. */
    QDateTime dateTime() const;
    enum DateTimeMode {
        Arrival, ///< dateTime() represents the desired arriva time.
        Departure ///< dateTime() represents the desired departure time.
    };
    DateTimeMode dateTimeMode() const;
    void setDeparutreTime(const QDateTime &dt);
    void setArrivalTime(const QDateTime &dt);

private:
    QExplicitlySharedDataPointer<JourneyRequestPrivate> d;
};

}

#endif // KPUBLICTRANSPORT_JOURNEYREQUEST_H
