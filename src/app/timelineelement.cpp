/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "timelineelement.h"

#include "publictransport.h"
#include "reservationmanager.h"
#include "timelinemodel.h"
#include "transfer.h"

#include <KItinerary/Event>
#include <KItinerary/LocationUtil>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>
#include <KItinerary/Visit>

#include <KPublicTransport/Journey>
#include <KPublicTransport/Stopover>

using namespace KItinerary;

static TimelineElement::ElementType elementType(const QVariant &res)
{
    if (JsonLd::isA<FlightReservation>(res)) { return TimelineElement::Flight; }
    if (JsonLd::isA<LodgingReservation>(res)) { return TimelineElement::Hotel; }
    if (JsonLd::isA<TrainReservation>(res)) { return TimelineElement::TrainTrip; }
    if (JsonLd::isA<BusReservation>(res)) { return TimelineElement::BusTrip; }
    if (JsonLd::isA<BoatReservation>(res)) { return TimelineElement::BoatTrip; }
    if (JsonLd::isA<FoodEstablishmentReservation>(res)) { return TimelineElement::Restaurant; }
    if (JsonLd::isA<TouristAttractionVisit>(res)) { return TimelineElement::TouristAttraction; }
    if (JsonLd::isA<EventReservation>(res)) { return TimelineElement::Event; }
    if (JsonLd::isA<RentalCarReservation>(res)) { return TimelineElement::CarRental; }
    return {};
}

TimelineElement::TimelineElement() = default;

TimelineElement::TimelineElement(TimelineModel *model, TimelineElement::ElementType type, const QDateTime &dateTime, const QVariant &data)
    : dt(dateTime)
    , elementType(type)
    , m_content(data)
    , m_model(model)
{
}

TimelineElement::TimelineElement(TimelineModel *model, const QString& resId, const QVariant& res, TimelineElement::RangeType rt)
    : dt(relevantDateTime(res, rt))
    , elementType(::elementType(res))
    , rangeType(rt)
    , m_content(resId)
    , m_model(model)
{
}

TimelineElement::TimelineElement(TimelineModel *model, const ::Transfer &transfer)
    : dt(transfer.alignment() == Transfer::Before ? transfer.anchorTime().addSecs(-transfer.anchorTimeDelta()) : transfer.anchorTime())
    , elementType(Transfer)
    , rangeType(SelfContained)
    , m_content(QVariant::fromValue(transfer))
    , m_model(model)
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
        case BoatTrip:
        case Restaurant:
        case TouristAttraction:
        case Event:
        case Hotel:
            return true;
        case Undefined:
        case TodayMarker:
        case TripGroup:
        case WeatherForecast:
        case LocationInfo:
        case Transfer:
            return false;
    }

    Q_UNREACHABLE();
    return false;
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

bool TimelineElement::isLocationChange() const
{
    if (isReservation()) {
        // ### can be done without reservation lookup for some of the reservation element types
        const auto res = m_model->m_resMgr->reservation(batchId());
        return LocationUtil::isLocationChange(res);
    }

    if (elementType == Transfer) {
        return m_content.value<::Transfer>().state() == Transfer::Selected;
    }

    return false;
}

bool TimelineElement::isTimeBoxed() const
{
    switch (elementType) {
        case Undefined:
        case TodayMarker:
        case TripGroup:
        case WeatherForecast:
        case LocationInfo:
            return false;
        case Transfer:
            return m_content.value<::Transfer>().state() == Transfer::Selected;
        case Flight:
        case TrainTrip:
        case BusTrip:
        case BoatTrip:
            return true;
        case Hotel:
        case CarRental:
            return false;
        case Restaurant:
        case TouristAttraction:
        case Event:
            return SortUtil::endDateTime(m_model->m_resMgr->reservation(batchId())).isValid();
    }
    return false;
}

bool TimelineElement::isInformational() const
{
    switch (elementType) {
        case Undefined:
        case TodayMarker:
        case TripGroup:
        case WeatherForecast:
        case LocationInfo:
            return true;
        case Transfer:
        case Flight:
        case TrainTrip:
        case BusTrip:
        case BoatTrip:
        case Hotel:
        case CarRental:
        case Restaurant:
        case TouristAttraction:
        case Event:
            return false;
    }
    Q_UNREACHABLE();
}

bool TimelineElement::isCanceled() const
{
    if (isReservation()) {
        const auto res = m_model->m_resMgr->reservation(batchId());
        return JsonLd::canConvert<Reservation>(res) && JsonLd::convert<Reservation>(res).reservationStatus() == Reservation::ReservationCancelled;
    }

    // TODO transfers with Disruption::NoService
    return false;
}

static KPublicTransport::Location destinationOfJourney(const KPublicTransport::Journey &jny)
{
    if (jny.sections().empty()) {
        return {};
    }
    const auto &sections = jny.sections();
    auto loc = sections.back().arrival().stopPoint();

    if (sections.size() == 1 || sections.back().mode() != KPublicTransport::JourneySection::Walking || sections.back().distance() > 1000) {
        return loc;
    }

    // the last section is a short walk, its arrival location might not have all the data we have on the previous stop
    const auto prevLoc = sections[sections.size() - 2].arrival().stopPoint();
    const auto locName = loc.name();

    // "anonymous" location with the coordinates as name
    if (std::none_of(locName.begin(), locName.end(), [](QChar c) { return c.isLetter(); }) && !prevLoc.name().isEmpty()) {
        return prevLoc;
    }

    // propagate address information from the last stop to the final destination
    if (loc.locality().isEmpty()) { loc.setLocality(prevLoc.locality()); }
    if (loc.region().isEmpty()) { loc.setRegion(prevLoc.region()); }
    if (loc.country().isEmpty()) { loc.setLocality(prevLoc.country()); }

    return loc;
}

QVariant TimelineElement::destination() const
{
    if (isReservation()) {
        const auto res = m_model->m_resMgr->reservation(batchId());
        if (LocationUtil::isLocationChange(res)) {
            return LocationUtil::arrivalLocation(res);
        }
        return LocationUtil::location(res);
    }

    if (elementType == Transfer) {
        const auto transfer = m_content.value<::Transfer>();
        if (transfer.state() == Transfer::Selected) {
            const auto loc = destinationOfJourney(transfer.journey());
            return loc.isEmpty() ? QVariant() : PublicTransport::placeFromLocation<KItinerary::Place>(loc);
        }
    }

    return {};
}

QDateTime TimelineElement::endDateTime() const
{
    if (isReservation()) {
        const auto res = m_model->m_resMgr->reservation(batchId());
        return SortUtil::endDateTime(res);
    }

    if (elementType == Transfer) {
        const auto transfer = m_content.value<::Transfer>();
        if (transfer.state() == Transfer::Selected) {
            return transfer.journey().scheduledArrivalTime();
        }
    }

    return {};
}

#include "moc_timelineelement.cpp"
