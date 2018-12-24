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

#include "location.h"

#include "datatypes_p.h"

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimeZone>

#include <cmath>

using namespace KPublicTransport;

namespace KPublicTransport {

class LocationPrivate : public QSharedData
{
public:
    QString name;
    float latitude = NAN;
    float longitude = NAN;
    QTimeZone timeZone;
    QHash<QString, QString> ids;
};

}

KPUBLICTRANSPORT_MAKE_GADGET(Location)

QString Location::name() const
{
    return d->name;
}

void Location::setName(const QString &name)
{
    d.detach();
    d->name = name;
}

float Location::latitude() const
{
    return d->latitude;
}

float Location::longitude() const
{
    return d->longitude;
}

void Location::setCoordinate(float latitude, float longitude)
{
    d.detach();
    d->latitude = latitude;
    d->longitude = longitude;
}

bool Location::hasCoordinate() const
{
    return !std::isnan(d->latitude) && !std::isnan(d->longitude);
}

QTimeZone Location::timeZone() const
{
    return d->timeZone;
}

void Location::setTimeZone(const QTimeZone &tz)
{
    d.detach();
    d->timeZone = tz;
}

QString Location::identifier(const QString &identifierType) const
{
    return d->ids.value(identifierType);
}

void Location::setIdentifier(const QString &identifierType, const QString &id)
{
    d.detach();
    d->ids.insert(identifierType, id);
}

QHash<QString, QString> Location::identifiers() const
{
    return d->ids;
}

bool Location::isSame(const Location &lhs, const Location &rhs)
{
    // ids
    for (auto it = lhs.identifiers().constBegin(); it != lhs.identifiers().constEnd(); ++it) {
        if (rhs.identifier(it.key()) == it.value() && !it.value().isEmpty()) {
            return true;
        }
    }

    // name
    if (lhs.name().compare(rhs.name(), Qt::CaseInsensitive) == 0) {
        return true;
    }

    // coordinates
    if (qFuzzyCompare(lhs.latitude(), rhs.latitude()) && qFuzzyCompare(lhs.longitude(), rhs.longitude())) {
        return true;
    }

    return false;
}

Location Location::merge(const Location &lhs, const Location &rhs)
{
    Location l(lhs);

    // merge identifiers
    for (auto it = rhs.identifiers().constBegin(); it != rhs.identifiers().constEnd(); ++it) {
        if (lhs.identifier(it.key()).isEmpty()) {
            l.setIdentifier(it.key(), it.value());
        }
    }

    return lhs;
}

// see https://en.wikipedia.org/wiki/Haversine_formula
int Location::distance(float lat1, float lon1, float lat2, float lon2)
{
    const auto degToRad = M_PI / 180.0;
    const auto earthRadius = 6371000.0; // in meters

    const auto d_lat = (lat1 - lat2) * degToRad;
    const auto d_lon = (lon1 - lon2) * degToRad;

    const auto a = pow(sin(d_lat / 2.0), 2) + cos(lat1 * degToRad) * cos(lat2 * degToRad) * pow(sin(d_lon / 2.0), 2);
    return 2.0 * earthRadius * atan2(sqrt(a), sqrt(1.0 - a));
}

QJsonObject Location::toJson(const Location &loc)
{
    QJsonObject obj;
    obj.insert(QLatin1String("name"), loc.name());
    obj.insert(QLatin1String("latitude"), loc.latitude());
    obj.insert(QLatin1String("longitude"), loc.longitude());
    if (loc.timeZone().isValid()) {
        obj.insert(QLatin1String("timezone"), QString::fromUtf8(loc.timeZone().id()));
    }

    QJsonObject ids;
    for (auto it = loc.d->ids.begin(); it != loc.d->ids.end(); ++it) {
        ids.insert(it.key(), it.value());
    }
    obj.insert(QLatin1String("identifier"), ids);

    return obj;
}

QJsonArray Location::toJson(const std::vector<Location> &locs)
{
    QJsonArray a;
    //a.reserve(locs.size());
    std::transform(locs.begin(), locs.end(), std::back_inserter(a), QOverload<const Location&>::of(&Location::toJson));
    return a;
}

static Location fromJsonObject(const QJsonObject &obj)
{
    Location loc;
    loc.setName(obj.value(QLatin1String("name")).toString());
    loc.setCoordinate(obj.value(QLatin1String("latitude")).toDouble(), obj.value(QLatin1String("longitude")).toDouble());
    const auto tz = obj.value(QLatin1String("timezone")).toString();
    if (!tz.isEmpty()) {
        loc.setTimeZone(QTimeZone(tz.toUtf8()));
    }

    const auto ids = obj.value(QLatin1String("identifier")).toObject();
    for (auto it = ids.begin(); it != ids.end(); ++it) {
        loc.setIdentifier(it.key(), it.value().toString());
    }

    return loc;
}

std::vector<Location> Location::fromJson(const QJsonValue &v)
{
    std::vector<Location> res;
    if (v.isArray()) {
        const auto a = v.toArray();
        res.reserve(a.size());
        std::transform(a.begin(), a.end(), std::back_inserter(res), [](const auto &v) { return fromJsonObject(v.toObject()); });
    } else if (v.isObject()) {
        res.push_back(fromJsonObject(v.toObject()));
    }
    return res;
}

#include "moc_location.cpp"
