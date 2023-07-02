/*
    SPDX-FileCopyrightText: 2019-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "publictransportmatcher.h"
#include "logging.h"
#include "publictransport.h"
#include "reservationhelper.h"

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

bool PublicTransportMatcher::isSameLine(const KPublicTransport::Line &lhs, const QString &trainName, const QString &trainNumber)
{
    KPublicTransport::Line rhs;
    rhs.setModeString(trainName);
    rhs.setName(trainNumber);
    return KPublicTransport::Line::isSame(lhs, rhs);
}

bool PublicTransportMatcher::isDepartureForReservation(const QVariant &res, const KPublicTransport::Stopover &dep)
{
    const auto lineData = ReservationHelper::lineNameAndNumber(res);
    return PublicTransportMatcher::isSameMode(res, dep.route().line().mode())
        && KItinerary::SortUtil::startDateTime(res) == dep.scheduledDepartureTime()
        && PublicTransportMatcher::isSameLine(dep.route().line(), lineData.first, lineData.second);
}

bool PublicTransportMatcher::isArrivalForReservation(const QVariant &res, const KPublicTransport::Stopover &arr)
{
    const auto lineData = ReservationHelper::lineNameAndNumber(res);
    return PublicTransportMatcher::isSameMode(res, arr.route().line().mode())
        && KItinerary::SortUtil::endDateTime(res) == arr.scheduledArrivalTime()
        && PublicTransportMatcher::isSameLine(arr.route().line(), lineData.first, lineData.second);
}

bool PublicTransportMatcher::isJourneyForReservation(const QVariant &res, const KPublicTransport::JourneySection &journey)
{
    const auto lineData = ReservationHelper::lineNameAndNumber(res);
    return PublicTransportMatcher::isSameMode(res, journey.route().line().mode())
        && KItinerary::SortUtil::startDateTime(res) == journey.scheduledDepartureTime()
        && PublicTransportMatcher::isSameLine(journey.route().line(), lineData.first, lineData.second);
}
