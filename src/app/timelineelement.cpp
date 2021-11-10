/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

TimelineElement::TimelineElement() = default;

TimelineElement::TimelineElement(TimelineElement::ElementType type, const QDateTime &dateTime, const QVariant &data)
    : dt(dateTime)
    , elementType(type)
    , m_content(data)
{
}

TimelineElement::TimelineElement(const QString& resId, const QVariant& res, TimelineElement::RangeType rt)
    : dt(relevantDateTime(res, rt))
    , elementType(::elementType(res))
    , rangeType(rt)
    , m_content(resId)
{
}

TimelineElement::TimelineElement(const ::Transfer &transfer)
    : dt(transfer.anchorTime())
    , elementType(Transfer)
    , rangeType(SelfContained)
    , m_content(QVariant::fromValue(transfer))
{
}

static bool operator<(TimelineElement::RangeType lhs, TimelineElement::RangeType rhs)
{
    static const int order_map[] = { 1, 0, 2 };
    return order_map[lhs] < order_map[rhs];
}

bool TimelineElement::operator<(const TimelineElement &other) const
{
    if (dt == other.dt) {
        if (rangeType == RangeEnd || other.rangeType == RangeEnd) {
            if (rangeType == RangeBegin || other.rangeType == RangeBegin) {
                return rangeType < other.rangeType;
            }
            return elementType > other.elementType;
        }
        return elementType < other.elementType;
    }
    return dt < other.dt;
}

bool TimelineElement::operator<(const QDateTime &otherDt) const
{
    return dt < otherDt;
}

bool TimelineElement::operator==(const TimelineElement &other) const
{
    if (elementType != other.elementType || rangeType != other.rangeType || dt != other.dt || batchId() != other.batchId()) {
        return false;
    }

    switch (elementType) {
        case Transfer:
        {
            const auto lhsT = m_content.value<::Transfer>();
            const auto rhsT = other.m_content.value<::Transfer>();
            return lhsT.alignment() == rhsT.alignment();
        }
        default:
            return true;
    }
}

bool TimelineElement::isReservation() const
{
    switch (elementType) {
        case Flight:
        case TrainTrip:
        case CarRental:
        case BusTrip:
        case Restaurant:
        case TouristAttraction:
        case Event:
        case Hotel:
            return true;
        default:
            return false;
    }
}

QString TimelineElement::batchId() const
{
    if (isReservation()) {
        return m_content.toString();
    }
    if (elementType == Transfer) {
        return m_content.value<::Transfer>().reservationId();
    }
    return {};
}

QVariant TimelineElement::content() const
{
    return m_content;
}

void TimelineElement::setContent(const QVariant& content)
{
    m_content = content;
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
