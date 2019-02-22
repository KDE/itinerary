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

#ifndef KPUBLICTRANSPORT_LOCATIONREQUEST_H
#define KPUBLICTRANSPORT_LOCATIONREQUEST_H

#include "kpublictransport_export.h"

#include <QSharedDataPointer>

namespace KPublicTransport {

class LocationRequestPrivate;

/** Describes a location search. */
class KPUBLICTRANSPORT_EXPORT LocationRequest
{
public:
    LocationRequest();
    LocationRequest(LocationRequest&&) noexcept;
    LocationRequest(const LocationRequest&);
    ~LocationRequest();
    LocationRequest& operator=(const LocationRequest&);

    float latitude() const;
    float longitude() const;
    /** Search by geo coordinate. */
    void setCoordinate(float lat, float lon);
    /** Returns true if a valid geo coordinate has been set. */
    bool hasCoordinate() const;

    QString name() const;
    /** Search by name or name fragment. */
    void setName(const QString &name);
    // TODO select full name or name fragment mode for auto-completion

    // unique string representation used for caching results
    QString cacheKey() const;

private:
    QExplicitlySharedDataPointer<LocationRequestPrivate> d;
};

}

#endif // KPUBLICTRANSPORT_LOCATIONREQUEST_H
