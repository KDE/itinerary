/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#include "timelineelement.h"
#include "transfer.h"

#include <KItinerary/Event>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>
#include <KItinerary/Visit>

using namespace KItinerary;

static TimelineElement::ElementType elementType(const QVariant &res)
{
    if (JsonLd::isA<FlightReservation>(res)) { return TimelineElement::Flight; }
    if (JsonLd::isA<LodgingReservation>(res)) { return TimelineElement::Hotel; }
    if (JsonLd::isA<TrainReservation>(res)) { return TimelineElement::TrainTrip; }
    if (JsonLd::isA<BusReservation>(res)) { return TimelineElement::BusTrip; }
    if (JsonLd::isA<FoodEstablishmentReservation>(res)) { return TimelineElement::Restaurant; }
    if (JsonLd::isA<TouristAttractionVisit>(res)) { return TimelineElement::TouristAttraction; }
    if (JsonLd::isA<EventReservation>(res)) { return TimelineElement::Event; }
    if (JsonLd::isA<RentalCarReservation>(res)) { return TimelineElement::CarRental; }
    return {};
}

TimelineElement::TimelineElement(TimelineElement::ElementType type, const QDateTime &dateTime, const QVariant &data)
    : content(data)
    , dt(dateTime)
    , elementType(type)
{
}

TimelineElement::TimelineElement(const QString& resId, const QVariant& res, TimelineElement::RangeType rt)
    : batchId(resId)
    , dt(relevantDateTime(res, rt))
    , elementType(::elementType(res))
    , rangeType(rt)
{
}

TimelineElement::TimelineElement(const ::Transfer &transfer)
    : content(QVariant::fromValue(transfer))
    , dt(transfer.anchorTime())
    , elementType(Transfer)
    , rangeType(SelfContained)
{
}

bool TimelineElement::operator<(const TimelineElement &other) const
{
    if (dt == other.dt) {
        return elementType < other.elementType;
    }
    return dt < other.dt;
}

QDateTime TimelineElement::relevantDateTime(const QVariant &res, TimelineElement::RangeType range)
{
    if (range == TimelineElement::RangeBegin || range == TimelineElement::SelfContained) {
        return SortUtil::startDateTime(res);
    }
    if (range == TimelineElement::RangeEnd) {
        return SortUtil::endDateTime(res);
    }

    return {};
}
