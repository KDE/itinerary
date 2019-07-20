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

#ifndef PUBLICTRANSPORT_H
#define PUBLICTRANSPORT_H

#include <QVariant>

namespace KItinerary {
class BusStation;
class TrainStation;
}

namespace KPublicTransport {
class JourneySection;
class Location;
}

/** Utilities to interface with KPublicTransport. */
namespace PublicTransport
{
    /** Obtain a KPublicTransport location object from a KItinerary place. */
    KPublicTransport::Location locationFromPlace(const QVariant &place);

    /** Merge information from @p location into the given train station.
     *  This assumes both sides refer to the same station, and @p loc merely provides additional information.
     *  In case of conflict @p station has precedence.
     */
    KItinerary::TrainStation mergeStation(KItinerary::TrainStation station, const KPublicTransport::Location &loc);

    /** Applies information from @p location to the given station.
     *  Unlike above, this does not assume both sides refer to the same location, and @p location has precedence.
     *  Data from @p station is only considered if both sides refer to the same location.
     */
    KItinerary::TrainStation applyStation(const KItinerary::TrainStation &station, const KPublicTransport::Location &loc);

    /** Update a reservation with a KPublictTransport::JourneySection.
     *  The journey section overwrites all corresponding properties of the reservation.
     *  This assumes that both sides are of a matching type (e.g. both referring to a train trip).
     *  @see isSameMode()
     */
    QVariant applyJourneySection(const QVariant &res, const KPublicTransport::JourneySection &section);

    /** Checks if the given reservation and journey section have a compatible mode of transportation. */
    bool isSameMode(const QVariant &res, const KPublicTransport::JourneySection &section);
}

/** Utility functions for interfacing with KPublicTransport from QML. */
class PublicTransportUtil
{
    Q_GADGET
public:
    /** Convert a KPublicTransport::Line::Mode enum value to an icon source or name
     *  for usage in Kirigami.Icon.
     */
    Q_INVOKABLE QString lineModeIcon(int lineMode);

    /** Create a KPublicTransport::DepartureRequest for the given KItinerary place. */
    Q_INVOKABLE QVariant departureRequestForPlace(const QVariant &place, const QDateTime &dt) const;

    /** Produces a short summary of the given attribution information. */
    Q_INVOKABLE QString attributionSummary(const QVariantList &attributions) const;
};

#endif // PUBLICTRANSPORT_H
