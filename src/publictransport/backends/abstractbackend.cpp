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

#include "abstractbackend.h"

#include <KPublicTransport/Location>

#include <QDebug>
#include <QPolygonF>

using namespace KPublicTransport;

AbstractBackend::AbstractBackend() = default;
AbstractBackend::~AbstractBackend() = default;

QString AbstractBackend::backendId() const
{
    return m_backendId;
}

void AbstractBackend::setBackendId(const QString& id)
{
    m_backendId = id;
}

bool AbstractBackend::isLocationExcluded(const Location &loc) const
{
    if (loc.hasCoordinate() && !m_geoFilter.isEmpty()) {
        return !m_geoFilter.containsPoint({loc.latitude(), loc.longitude()}, Qt::WindingFill);
    }
    return false;
}

void AbstractBackend::setGeoFilter(const QPolygonF &poly)
{
    m_geoFilter = poly;
}

bool AbstractBackend::isSecure() const
{
    return false;
}

bool AbstractBackend::queryDeparture(DepartureReply *reply, QNetworkAccessManager *nam) const
{
    Q_UNUSED(reply);
    Q_UNUSED(nam);
    return false;
}

bool AbstractBackend::queryJourney(JourneyReply *reply, QNetworkAccessManager *nam) const
{
    Q_UNUSED(reply);
    Q_UNUSED(nam);
    return false;
}
