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

#include "departurerequest.h"

#include <KPublicTransport/Location>

#include <QDateTime>
#include <QSharedData>

using namespace KPublicTransport;

namespace KPublicTransport {
class DepartureRequestPrivate : public QSharedData {
public:
    Location stop;
    QDateTime dateTime;
    DepartureRequest::Mode mode = DepartureRequest::QueryDeparture;
};
}

DepartureRequest::DepartureRequest()
    : d(new DepartureRequestPrivate)
{
}

DepartureRequest::DepartureRequest(const Location &stop)
    : d(new DepartureRequestPrivate)
{
    d->stop = stop;
}

DepartureRequest::DepartureRequest(DepartureRequest&&) noexcept = default;
DepartureRequest::DepartureRequest(const DepartureRequest&) = default;
DepartureRequest::~DepartureRequest() = default;
DepartureRequest& DepartureRequest::operator=(const DepartureRequest&) = default;

Location DepartureRequest::stop() const
{
    return d->stop;
}

QDateTime DepartureRequest::dateTime() const
{
    if (!d->dateTime.isValid()) {
        d->dateTime = QDateTime::currentDateTime();
    }
    return d->dateTime;
}

void DepartureRequest::setDateTime(const QDateTime &dt)
{
    d.detach();
    d->dateTime = dt;
}

DepartureRequest::Mode DepartureRequest::mode() const
{
    return d->mode;
}

void DepartureRequest::setMode(DepartureRequest::Mode mode)
{
    d.detach();
    d->mode = mode;
}
