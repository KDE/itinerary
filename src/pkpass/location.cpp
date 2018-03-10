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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "location.h"

#include <QJsonObject>

#include <cmath>

using namespace KPkPass;

namespace KPkPass {
class LocationPrivate {
public:
    QJsonObject obj;
};
}

Location::Location()
    : d(new LocationPrivate)
{
}

Location::Location(const QJsonObject &obj)
    : d(new LocationPrivate)
{
    d->obj = obj;
}

Location::~Location() = default;

double Location::altitude() const
{
    return d->obj.value(QLatin1String("altitude")).toDouble(NAN);
}

double Location::latitude() const
{
    return d->obj.value(QLatin1String("latitude")).toDouble(NAN);
}

double Location::longitude() const
{
    return d->obj.value(QLatin1String("longitude")).toDouble(NAN);
}

QString Location::relevantText() const
{
    return d->obj.value(QLatin1String("relevantText")).toString();
}
