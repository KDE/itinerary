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
    // TODO

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

    // TODO merge identifiers

    return lhs;
}

#include "moc_location.cpp"
