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
    QDateTime scheduledDepartureTime;
    QDateTime expectedDepartureTime;
    QDateTime scheduledArrivalTime;
    QDateTime expectedArrivalTime;
    Location from;
    Location to;
    Route route;
    QString scheduledDeparturePlatform;
    QString expectedDeparturePlatform;
    QString scheduledArrivalPlatform;
    QString expectedArrivalPlatform;
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

QDateTime JourneySection::scheduledDepartureTime() const
{
    return d->scheduledDepartureTime;
}

void JourneySection::setScheduledDepartureTime(const QDateTime &dt)
{
    d.detach();
    d->scheduledDepartureTime = dt;
}

QDateTime JourneySection::expectedDepartureTime() const
{
    return d->expectedDepartureTime;
}

void JourneySection::setExpectedDepartureTime(const QDateTime &dt)
{
    d.detach();
    d->expectedDepartureTime = dt;
}

bool JourneySection::hasExpectedDepartureTime() const
{
    return d->expectedDepartureTime.isValid();
}

int JourneySection::departureDelay() const
{
    if (hasExpectedDepartureTime()) {
        return d->scheduledDepartureTime.secsTo(d->expectedDepartureTime) / 60;
    }
    return 0;
}

QDateTime JourneySection::scheduledArrivalTime() const
{
    return d->scheduledArrivalTime;
}

void JourneySection::setScheduledArrivalTime(const QDateTime &dt)
{
    d.detach();
    d->scheduledArrivalTime = dt;
}

QDateTime JourneySection::expectedArrivalTime() const
{
    return d->expectedArrivalTime;
}

void JourneySection::setExpectedArrivalTime(const QDateTime &dt)
{
    d.detach();
    d->expectedArrivalTime = dt;
}

bool JourneySection::hasExpectedArrivalTime() const
{
    return d->expectedArrivalTime.isValid();
}

int JourneySection::arrivalDelay() const
{
    if (hasExpectedArrivalTime()) {
        return d->scheduledArrivalTime.secsTo(d->expectedArrivalTime) / 60;
    }
    return 0;
}

int JourneySection::duration() const
{
    return d->scheduledDepartureTime.secsTo(d->scheduledArrivalTime);
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

QString JourneySection::scheduledDeparturePlatform() const
{
    return d->scheduledDeparturePlatform;
}

void JourneySection::setScheduledDeparturePlatform(const QString &platform)
{
    d.detach();
    d->scheduledDeparturePlatform = platform;
}

QString JourneySection::expectedDeparturePlatform() const
{
    return d->expectedDeparturePlatform;
}

void JourneySection::setExpectedDeparturePlatform(const QString &platform)
{
    d.detach();
    d->expectedDeparturePlatform = platform;
}

bool JourneySection::hasExpectedDeparturePlatform() const
{
    return !d->expectedDeparturePlatform.isEmpty();
}

bool JourneySection::departurePlatformChanged() const
{
    return hasExpectedDeparturePlatform() && d->expectedDeparturePlatform != d->scheduledDeparturePlatform;
}

QString JourneySection::scheduledArrivalPlatform() const
{
    return d->scheduledArrivalPlatform;
}

void JourneySection::setScheduledArrivalPlatform(const QString &platform)
{
    d.detach();
    d->scheduledArrivalPlatform = platform;
}

QString JourneySection::expectedArrivalPlatform() const
{
    return d->expectedArrivalPlatform;
}

void JourneySection::setExpectedArrivalPlatform(const QString &platform)
{
    d.detach();
    d->expectedArrivalPlatform = platform;
}

bool JourneySection::hasExpectedArrivalPlatform() const
{
    return !d->expectedArrivalPlatform.isEmpty();
}

bool JourneySection::arrivalPlatformChanged() const
{
    return hasExpectedArrivalPlatform() && d->scheduledArrivalPlatform != d->expectedArrivalPlatform;
}

bool JourneySection::isSame(const JourneySection &lhs, const JourneySection &rhs)
{
    return lhs.d->mode == rhs.d->mode
        && lhs.d->scheduledDepartureTime == rhs.d->scheduledDepartureTime
        && lhs.d->scheduledArrivalTime == rhs.d->scheduledArrivalTime
        && Location::isSame(lhs.d->from, rhs.d->from)
        && Location::isSame(lhs.d->to, rhs.d->to)
        && Route::isSame(lhs.d->route, rhs.d->route);
        // ### platforms relevant?
}

JourneySection JourneySection::merge(const JourneySection &lhs, const JourneySection &rhs)
{
    auto res = lhs;
    if (!res.expectedDepartureTime().isValid()) {
        res.setExpectedDepartureTime(rhs.expectedDepartureTime());
    }
    if (res.expectedDeparturePlatform().isEmpty()) {
        res.setExpectedDeparturePlatform(rhs.expectedDeparturePlatform());
    }
    if (!res.expectedArrivalTime().isValid()) {
        res.setExpectedArrivalTime(rhs.expectedArrivalTime());
    }
    if (res.expectedArrivalPlatform().isEmpty()) {
        res.setExpectedArrivalPlatform(rhs.expectedArrivalPlatform());
    }
    res.setFrom(Location::merge(lhs.from(), rhs.from()));
    res.setTo(Location::merge(lhs.to(), rhs.to()));
    res.setRoute(Route::merge(lhs.route(), rhs.route()));

    return res;
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

QDateTime Journey::scheduledDepartureTime() const
{
    if (!d->sections.empty()) {
        return d->sections.front().scheduledDepartureTime();
    }
    return {};
}

QDateTime Journey::scheduledArrivalTime() const
{
    if (!d->sections.empty()) {
        return d->sections.back().scheduledArrivalTime();
    }
    return {};
}

int Journey::duration() const
{
    return scheduledDepartureTime().secsTo(scheduledArrivalTime());
}

int Journey::numberOfChanges() const
{
    return std::count_if(d->sections.begin(), d->sections.end(), [](const auto &section) { return section.mode() == JourneySection::PublicTransport; });
}

bool Journey::isSame(const Journey &lhs, const Journey &rhs)
{
    // ### we can make this more clever by ignoring transit elements for example
    // doing that will need changes below too!
    if (lhs.sections().size() != rhs.sections().size()) {
        return false;
    }
    return std::equal(lhs.sections().begin(), lhs.sections().end(), rhs.sections().begin(), [](const auto &lhs, const auto &rhs) {
        return JourneySection::isSame(lhs, rhs);
    });
}

Journey Journey::merge(const Journey &lhs, const Journey &rhs)
{
    // ### see above
    std::vector<JourneySection> sections;
    sections.reserve(lhs.sections().size());
    for (auto lit = lhs.sections().begin(), rit = rhs.sections().begin(); lit != lhs.sections().end(); ++lit, ++rit) {
        sections.push_back(JourneySection::merge(*lit, *rit));
    }

    Journey res;
    res.setSections(std::move(sections));
    return res;
}

#include "moc_journey.cpp"
