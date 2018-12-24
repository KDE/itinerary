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

#include "locationrequest.h"

#include <QSharedData>

#include <cmath>

using namespace KPublicTransport;

namespace KPublicTransport {
class LocationRequestPrivate : public QSharedData {
public:
    QString name;
    float lat = NAN;
    float lon = NAN;
};
}

LocationRequest::LocationRequest()
    : d(new LocationRequestPrivate)
{
}

LocationRequest::LocationRequest(LocationRequest&&) noexcept = default;
LocationRequest::LocationRequest(const LocationRequest&) = default;
LocationRequest::~LocationRequest() = default;
LocationRequest& LocationRequest::operator=(const LocationRequest&) = default;

float LocationRequest::latitude() const
{
    return d->lat;
}

float LocationRequest::longitude() const
{
    return d->lon;
}

void LocationRequest::setCoordinate(float lat, float lon)
{
    d.detach();
    d->lat = lat;
    d->lon = lon;
}

bool LocationRequest::hasCoordinate() const
{
    return !std::isnan(d->lat) && !std::isnan(d->lon);
}

QString LocationRequest::name() const
{
    return d->name;
}

void LocationRequest::setName(const QString &name)
{
    d->name = name;
}

QString LocationRequest::cacheKey() const
{
    QString normalizedName;
    normalizedName.reserve(d->name.size());
    for (const auto c : qAsConst(d->name)) {
        if (c.isLetter() || c.isDigit()) {
            normalizedName.push_back(c.toCaseFolded());
        }
    }

    if (hasCoordinate()) {
        return QString::number((int)(latitude() * 1000000)) + QLatin1Char('x') + QString::number((int)(longitude() * 1000000))
            + QLatin1Char('_') + normalizedName;
    } else {
        return QLatin1String("nanxnan_") + normalizedName;
    }
}
