/*
    SPDX-FileCopyrightText: 2019-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PUBLICTRANSPORTMATCHER_H
#define PUBLICTRANSPORTMATCHER_H

#include <KPublicTransport/Line>

namespace KPublicTransport
{
class Journey;
class JourneySection;
class Stopover;
}

class QVariant;

/** Methods to compare/match KPublicTransport types and KItinerary types. */
namespace PublicTransportMatcher
{

/** Checks if the given reservation and journey section have a compatible mode of transportation. */
bool isSameMode(const QVariant &res, KPublicTransport::Line::Mode mode);
bool isSameMode(const QVariant &res, const KPublicTransport::JourneySection &section);

/** Checks whether a given route and a given train name/number from a reservation refer to the same thing. */
bool isSameRoute(const KPublicTransport::Route &lhs, const QString &trainName, const QString &trainNumber);

/** Check whether a given reservation matches the respective departure/arrival/journey object. */
bool isDepartureForReservation(const QVariant &res, const KPublicTransport::Stopover &dep);
bool isArrivalForReservation(const QVariant &res, const KPublicTransport::Stopover &arr);
bool isJourneyForReservation(const QVariant &res, const KPublicTransport::JourneySection &journey);

/** Returns a sub-section of @p journey that matches @p res, or an empty journey otherwise. */
KPublicTransport::JourneySection subJourneyForReservation(const QVariant &res, const KPublicTransport::JourneySection &journey);

}

#endif // PUBLICTRANSPORTMATCHER_H
