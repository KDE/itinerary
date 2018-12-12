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

#include "journey.h"
#include "datatypes_p.h"

#include <QVariant>

using namespace KPublicTransport;

namespace KPublicTransport {

class JourneySectionPrivate : public QSharedData
{
public:
    JourneySection::Mode mode = JourneySection::Invalid;
    QDateTime departureTime;
    QDateTime arrivalTime;
    Location from;
    Location to;
    Route route;
};

class JourneyPrivate : public QSharedData
{
public:
    std::vector<JourneySection> sections;
};

}

KPUBLICTRANSPORT_MAKE_GADGET(JourneySection)

JourneySection::Mode JourneySection::mode() const
{
    return d->mode;
}

void JourneySection::setMode(JourneySection::Mode mode)
{
    d.detach();
    d->mode = mode;
}

QDateTime JourneySection::departureTime() const
{
    return d->departureTime;
}

void JourneySection::setDepartureTime(const QDateTime &dt)
{
    d.detach();
    d->departureTime = dt;
}

QDateTime JourneySection::arrivalTime() const
{
    return d->arrivalTime;
}

void JourneySection::setArrivalTime(const QDateTime &dt)
{
    d.detach();
    d->arrivalTime = dt;
}

int JourneySection::duration() const
{
    return d->departureTime.secsTo(d->arrivalTime);
}

Location JourneySection::from() const
{
    return d->from;
}

void JourneySection::setFrom(const Location &from)
{
    d.detach();
    d->from = from;
}

Location JourneySection::to() const
{
    return d->to;
}

void JourneySection::setTo(const Location &to)
{
    d.detach();
    d->to = to;
}

Route JourneySection::route() const
{
    return d->route;
}

void JourneySection::setRoute(const Route &route)
{
    d.detach();
    d->route = route;
}


KPUBLICTRANSPORT_MAKE_GADGET(Journey)

const std::vector<JourneySection>& Journey::sections() const
{
    return d->sections;
}

std::vector<JourneySection>&& Journey::takeSections()
{
    return std::move(d->sections);
}

void Journey::setSections(std::vector<JourneySection> &&sections)
{
    d.detach();
    d->sections = std::move(sections);
}

QVariantList Journey::sectionsVariant() const
{
    QVariantList l;
    l.reserve(d->sections.size());
    std::transform(d->sections.begin(), d->sections.end(), std::back_inserter(l), [](const auto &sec) { return QVariant::fromValue(sec); });
    return l;
}

QDateTime KPublicTransport::Journey::departureTime() const
{
    if (!d->sections.empty()) {
        return d->sections.front().departureTime();
    }
    return {};
}

QDateTime Journey::arrivalTime() const
{
    if (!d->sections.empty()) {
        return d->sections.back().arrivalTime();
    }
    return {};
}

int Journey::duration() const
{
    return departureTime().secsTo(arrivalTime());
}

#include "moc_journey.cpp"
