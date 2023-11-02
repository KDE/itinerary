/*
    SPDX-FileCopyrightText: 2019-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "publictransportmatcher.h"
#include "logging.h"
#include "publictransport.h"
#include "reservationhelper.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>

#include <KPublicTransport/Journey>
#include <KPublicTransport/Stopover>

bool PublicTransportMatcher::isSameMode(const QVariant &res, KPublicTransport::Line::Mode mode)
{
    using namespace KPublicTransport;

    if (KItinerary::JsonLd::isA<KItinerary::TrainReservation>(res)) {
        return PublicTransport::isTrainMode(mode);
    } else if (KItinerary::JsonLd::isA<KItinerary::BusReservation>(res)) {
        return PublicTransport::isBusMode(mode);
    } else if (KItinerary::JsonLd::isA<KItinerary::FlightReservation>(res)) {
        return mode == Line::Air;
    } else if (KItinerary::JsonLd::isA<KItinerary::BoatReservation>(res)) {
        return mode == Line::Boat || mode == Line::Ferry;
    } else if (res.isValid()) {
        qCWarning(Log) << "unexpected reservation type!?" << res;
    }

    return false;
}

bool PublicTransportMatcher::isSameMode(const QVariant &res, const KPublicTransport::JourneySection &section)
{
    return isSameMode(res, section.route().line().mode());
}

bool PublicTransportMatcher::isSameRoute(const KPublicTransport::Route &lhs, const QString &trainName, const QString &trainNumber)
{
    KPublicTransport::Line rhs;
    rhs.setModeString(trainName);
    rhs.setName(trainNumber);
    if (KPublicTransport::Line::isSame(lhs.line(), rhs)) {
        return true;
    }
    if (lhs.name().size() >= 3) {
        rhs.setName(QString(trainName + QLatin1Char(' ') + trainNumber).trimmed());
        auto lhsLine = lhs.line();
        lhsLine.setModeString(QString());
        lhsLine.setName(lhs.name());
        return KPublicTransport::Line::isSame(lhsLine, rhs);
    }
    return false;
}

bool PublicTransportMatcher::isDepartureForReservation(const QVariant &res, const KPublicTransport::Stopover &dep)
{
    const auto lineData = ReservationHelper::lineNameAndNumber(res);
    return PublicTransportMatcher::isSameMode(res, dep.route().line().mode())
        && KItinerary::SortUtil::startDateTime(res) == dep.scheduledDepartureTime()
        && PublicTransportMatcher::isSameRoute(dep.route(), lineData.first, lineData.second);
}

bool PublicTransportMatcher::isArrivalForReservation(const QVariant &res, const KPublicTransport::Stopover &arr)
{
    const auto lineData = ReservationHelper::lineNameAndNumber(res);
    return PublicTransportMatcher::isSameMode(res, arr.route().line().mode())
        && KItinerary::SortUtil::endDateTime(res) == arr.scheduledArrivalTime()
        && PublicTransportMatcher::isSameRoute(arr.route(), lineData.first, lineData.second);
}

bool PublicTransportMatcher::isJourneyForReservation(const QVariant &res, const KPublicTransport::JourneySection &journey)
{
    const auto lineData = ReservationHelper::lineNameAndNumber(res);
    return PublicTransportMatcher::isSameMode(res, journey.route().line().mode())
        && KItinerary::SortUtil::startDateTime(res) == journey.scheduledDepartureTime()
        && PublicTransportMatcher::isSameRoute(journey.route(), lineData.first, lineData.second);
}

/* Compare times without assuming times without a timezone are in the current time zone
 * (they might be local to the destination instead).
 */
static bool isSameDateTime(const QDateTime &lhs, const QDateTime &rhs)
{
    if (lhs == rhs) {
        return true;
    }
    if (lhs.timeSpec() == Qt::LocalTime && rhs.timeSpec() != Qt::LocalTime) {
        QDateTime dt(rhs);
        dt.setTimeSpec(Qt::LocalTime);
        return lhs == dt;
    }
    if (lhs.timeSpec() != Qt::LocalTime && rhs.timeSpec() == Qt::LocalTime) {
        QDateTime dt(lhs);
        dt.setTimeSpec(Qt::LocalTime);
        return dt == rhs;
    }
    return false;
}

static bool isSameDeparture(const QVariant &depLoc, const QVariant &res, const QDateTime &depTime, const KPublicTransport::Stopover &stop)
{
    return isSameDateTime(depTime, stop.scheduledDepartureTime())
        && KPublicTransport::Location::isSame(PublicTransport::locationFromPlace(depLoc, res), stop.stopPoint());
}

static bool isSameArrival(const QVariant &arrLoc, const QVariant &res, const QDateTime &arrTime, const KPublicTransport::Stopover &stop)
{
    return isSameDateTime(arrTime, stop.scheduledArrivalTime())
        && KPublicTransport::Location::isSame(PublicTransport::locationFromPlace(arrLoc, res), stop.stopPoint());
}

KPublicTransport::JourneySection PublicTransportMatcher::subJourneyForReservation(const QVariant &res, const KPublicTransport::JourneySection &journey)
{
    const auto lineData = ReservationHelper::lineNameAndNumber(res);
    if (!PublicTransportMatcher::isSameMode(res, journey.route().line().mode()) ||
        !PublicTransportMatcher::isSameRoute(journey.route(), lineData.first, lineData.second)) {
        return {};
    }

    KPublicTransport::JourneySection result(journey);
    auto stopovers = result.takeIntermediateStops();

    auto it = stopovers.begin();
    if (!isSameDeparture(KItinerary::LocationUtil::departureLocation(res), res, KItinerary::SortUtil::startDateTime(res), journey.departure())) {
        for (;it != stopovers.end(); ++it) {
            if (isSameDeparture(KItinerary::LocationUtil::departureLocation(res), res, KItinerary::SortUtil::startDateTime(res), *it)) {
                result.setDeparture(*it);
                break;
            }
        }
    }
    if (it == stopovers.end()) {
        return {};
    }
    ++it;
    auto it2 = it;
    for (;it2 != stopovers.end(); ++it2) {
        if (isSameArrival(KItinerary::LocationUtil::arrivalLocation(res), res, KItinerary::SortUtil::endDateTime(res), *it2)) {
            result.setArrival(*it2);
            break;
        }
    }
    if (it2 == stopovers.end() && !isSameArrival(KItinerary::LocationUtil::arrivalLocation(res), res, KItinerary::SortUtil::endDateTime(res), journey.arrival())) {
        return {};
    }

    result.setIntermediateStops({it, it2});
    return result;
}
