/*
    SPDX-FileCopyrightText: 2019-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "publictransportmatcher.h"
#include "datetimehelper.h"
#include "logging.h"
#include "publictransport.h"
#include "reservationhelper.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>

#include <KPublicTransport/Journey>
#include <KPublicTransport/Stopover>

#include <QTimeZone>

bool PublicTransportMatcher::isSameMode(const QVariant &res, KPublicTransport::Line::Mode mode)
{
    using namespace KPublicTransport;

    if (KItinerary::JsonLd::isA<KItinerary::TrainReservation>(res)) {
        return KPublicTransport::Line::modeIsRailBound(mode);
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
    return PublicTransportMatcher::isSameMode(res, dep.route().line().mode()) && KItinerary::SortUtil::startDateTime(res) == dep.scheduledDepartureTime()
        && PublicTransportMatcher::isSameRoute(dep.route(), lineData.first, lineData.second);
}

bool PublicTransportMatcher::isArrivalForReservation(const QVariant &res, const KPublicTransport::Stopover &arr)
{
    const auto lineData = ReservationHelper::lineNameAndNumber(res);
    return PublicTransportMatcher::isSameMode(res, arr.route().line().mode()) && KItinerary::SortUtil::endDateTime(res) == arr.scheduledArrivalTime()
        && PublicTransportMatcher::isSameRoute(arr.route(), lineData.first, lineData.second);
}

bool PublicTransportMatcher::isJourneyForReservation(const QVariant &res, const KPublicTransport::JourneySection &journey)
{
    const auto lineData = ReservationHelper::lineNameAndNumber(res);
    return PublicTransportMatcher::isSameMode(res, journey.route().line().mode())
        && KItinerary::SortUtil::startDateTime(res) == journey.scheduledDepartureTime()
        && KItinerary::SortUtil::endDateTime(res) == journey.scheduledArrivalTime()
        && PublicTransportMatcher::isSameRoute(journey.route(), lineData.first, lineData.second);
}

static bool isSameDeparture(const QVariant &depLoc, const QDateTime &depTime, const KPublicTransport::Stopover &stop)
{
    return DateTimeHelper::isSameDateTime(depTime, stop.scheduledDepartureTime())
        && KPublicTransport::Location::isSame(PublicTransport::locationFromPlace(depLoc), stop.stopPoint());
}

static bool isSameArrival(const QVariant &arrLoc, const QDateTime &arrTime, const KPublicTransport::Stopover &stop)
{
    return DateTimeHelper::isSameDateTime(arrTime, stop.scheduledArrivalTime())
        && KPublicTransport::Location::isSame(PublicTransport::locationFromPlace(arrLoc), stop.stopPoint());
}

KPublicTransport::JourneySection PublicTransportMatcher::subJourneyForReservation(const QVariant &res, const KPublicTransport::JourneySection &journey)
{
    const auto lineData = ReservationHelper::lineNameAndNumber(res);
    if (!PublicTransportMatcher::isSameMode(res, journey.route().line().mode())
        || !PublicTransportMatcher::isSameRoute(journey.route(), lineData.first, lineData.second)) {
        return {};
    }

    const auto sameDep = isSameDeparture(KItinerary::LocationUtil::departureLocation(res), KItinerary::SortUtil::startDateTime(res), journey.departure());
    const auto sameArr = isSameArrival(KItinerary::LocationUtil::arrivalLocation(res), KItinerary::SortUtil::endDateTime(res), journey.arrival());
    if (sameDep && sameArr) {
        return journey;
    }

    KPublicTransport::JourneySection result(journey);
    auto stopovers = result.takeIntermediateStops();

    auto it = stopovers.begin();
    if (!sameDep) {
        for (; it != stopovers.end(); ++it) {
            if (isSameDeparture(KItinerary::LocationUtil::departureLocation(res), KItinerary::SortUtil::startDateTime(res), *it)) {
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
    for (; it2 != stopovers.end(); ++it2) {
        if (isSameArrival(KItinerary::LocationUtil::arrivalLocation(res), KItinerary::SortUtil::endDateTime(res), *it2)) {
            result.setArrival(*it2);
            break;
        }
    }
    if (it2 == stopovers.end() && !sameArr) {
        return {};
    }

    result.setIntermediateStops({it, it2});
    return result;
}
