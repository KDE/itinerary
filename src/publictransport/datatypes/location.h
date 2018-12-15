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

#ifndef KPUBLICTRANSPORT_LOCATION_H
#define KPUBLICTRANSPORT_LOCATION_H

#include "datatypes.h"

class QTimeZone;

namespace KPublicTransport {

class LocationPrivate;

/** A location. */
class Location
{
    KPUBLICTRANSPORT_GADGET(Location)
    /** Human-readable name of the location. */
    Q_PROPERTY(QString name READ name WRITE setName)

    // TODO: type, coords, id, address

public:
    QString name() const;
    void setName(const QString &name);

    float latitude() const;
    float longitude() const;
    void setCoordinate(float latitude, float longitude);

    /** The timezone this location is in, if known. */
    QTimeZone timeZone() const;
    void setTimeZone(const QTimeZone &tz);

    /** Location identifiers. */
    QString identifier(const QString &identifierType) const;
    void setIdentifier(const QString &identifierType, const QString &id);
};

}

Q_DECLARE_METATYPE(KPublicTransport::Location)

#endif // KPUBLICTRANSPORT_LOCATION_H
