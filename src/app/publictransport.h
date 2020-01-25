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

#include <KItinerary/LocationUtil>
#include <KItinerary/MergeUtil>
#include <KItinerary/Place>

#include <KPublicTransport/Location>

namespace KPublicTransport {
class JourneySection;
}

/** Utilities to interface with KPublicTransport. */
class PublicTransport
{
    Q_GADGET

public:
    /** Obtain a KPublicTransport location object from a KItinerary place.
     *  @param place The place to convert to a location
     *  @param reservation The enclosing reservation object of this place.
     *  This matters to decide how to interpret the name/address/etc of the place.
     */
    static KPublicTransport::Location locationFromPlace(const QVariant &place, const QVariant &reservation);

    /** Create a KItinerary place type from the given KPublicTransport::Location. */
    template <typename T>
    static T placeFromLocation(const KPublicTransport::Location &loc);

    /** Merge information from @p location into the given train station.
     *  This assumes both sides refer to the same station, and @p loc merely provides additional information.
     *  In case of conflict @p station has precedence.
     */
    static KItinerary::TrainStation mergeStation(const KItinerary::TrainStation &station, const KPublicTransport::Location &loc);

    /** Applies information from @p location to the given KItinerary place.
     *  Unlike above, this does not assume both sides refer to the same location, and @p location has precedence.
     *  Data from @p station is only considered if both sides refer to the same location.
     */
    template <typename T>
    static T updateToLocation(const T &place, const KPublicTransport::Location &loc);

    /** Creates a reservation from the given journey section. */
    static QVariant reservationFromJourneySection(const KPublicTransport::JourneySection &section);

    /** Update a reservation with a KPublictTransport::JourneySection.
     *  The journey section overwrites all corresponding properties of the reservation.
     *  This assumes that both sides are of a matching type (e.g. both referring to a train trip).
     *  @see isSameMode()
     */
    static QVariant applyJourneySection(const QVariant &res, const KPublicTransport::JourneySection &section);

    /** Checks if the given reservation and journey section have a compatible mode of transportation. */
    static bool isSameMode(const QVariant &res, const KPublicTransport::JourneySection &section);

    /** Convert a KPublicTransport::Line::Mode enum value to an icon source or name
     *  for usage in Kirigami.Icon.
     */
    Q_INVOKABLE QString lineModeIcon(int lineMode);

    /** Create a KPublicTransport::DepartureRequest for the given KItinerary place. */
    Q_INVOKABLE QVariant departureRequestForPlace(const QVariant &place, const QDateTime &dt) const;

    /** Produces a short summary of the given attribution information. */
    Q_INVOKABLE QString attributionSummary(const QVariantList &attributions) const;

    /** Returns @c true when we want to highlight @p sections due to looking problematic. */
    Q_INVOKABLE bool warnAboutSection(const KPublicTransport::JourneySection &section) const;

private:
    // for use by the template code
    static QString idenfitierFromLocation(const KPublicTransport::Location &loc);
    static KItinerary::PostalAddress addressFromLocation(const KPublicTransport::Location &loc);
};


template <typename T>
inline T PublicTransport::placeFromLocation(const KPublicTransport::Location &loc)
{
    T place;
    place.setName(loc.name());
    if (loc.hasCoordinate()) {
        place.setGeo(KItinerary::GeoCoordinates{loc.latitude(), loc.longitude()});
    }
    place.setIdentifier(idenfitierFromLocation(loc));
    place.setAddress(addressFromLocation(loc));
    return place;
}

template <typename T>
inline T PublicTransport::updateToLocation(const T &place, const KPublicTransport::Location &loc)
{
    using namespace KItinerary;

    auto newPlace = placeFromLocation<T>(loc);
    if (LocationUtil::isSameLocation(place, newPlace)) {
        return MergeUtil::merge(place, newPlace).template value<T>();
    }

    return newPlace;
}

#endif // PUBLICTRANSPORT_H
