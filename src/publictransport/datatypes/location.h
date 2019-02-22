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

class QJsonArray;
class QJsonObject;
class QJsonValue;
class QTimeZone;
template <typename K, typename T> class QHash;

namespace KPublicTransport {

class LocationPrivate;

/** A location. */
class KPUBLICTRANSPORT_EXPORT Location
{
    KPUBLICTRANSPORT_GADGET(Location)
    /** Human-readable name of the location. */
    Q_PROPERTY(QString name READ name WRITE setName)

    Q_PROPERTY(float latitude READ latitude WRITE setLatitude)
    Q_PROPERTY(float longitude READ longitude WRITE setLongitude)

    // TODO: type, id, address

public:
    QString name() const;
    void setName(const QString &name);

    float latitude() const;
    void setLatitude(float latitude);
    float longitude() const;
    void setLongitude(float longitude);
    void setCoordinate(float latitude, float longitude);
    bool hasCoordinate() const;

    /** The timezone this location is in, if known. */
    QTimeZone timeZone() const;
    void setTimeZone(const QTimeZone &tz);

    /** Location identifiers. */
    QString identifier(const QString &identifierType) const;
    void setIdentifier(const QString &identifierType, const QString &id);
    QHash<QString, QString> identifiers() const;

    /** Checks if to instances refer to the same location (which does not necessarily mean they are exactly equal). */
    static bool isSame(const Location &lhs, const Location &rhs);

    /** Merge two departure instances.
     *  This assumes isSame(lhs, rhs) and tries to preserve the most detailed information.
     */
    static Location merge(const Location &lhs, const Location &rhs);

    /** Compute the distance between two geo coordinates, in meters. */
    static int distance(float lat1, float lon1, float lat2, float lon2);

    /** Serializes one Location object to JSON. */
    static QJsonObject toJson(const Location &loc);
    /** Serializes an array of Location objects to JSON. */
    static QJsonArray toJson(const std::vector<Location> &locs);
    /** Deserialize a Location object from JSON. */
    static Location fromJson(const QJsonObject &obj);
    /** Dezerializes an array Location objects from JSON. */
    static std::vector<Location> fromJson(const QJsonArray &a);

};

}

Q_DECLARE_METATYPE(KPublicTransport::Location)

#endif // KPUBLICTRANSPORT_LOCATION_H
