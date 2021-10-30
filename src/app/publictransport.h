/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PUBLICTRANSPORT_H
#define PUBLICTRANSPORT_H

#include <QVariant>

#include <KItinerary/LocationUtil>
#include <KItinerary/MergeUtil>
#include <KItinerary/Place>

#include <KPublicTransport/Location>
#include <KPublicTransport/Line>
#include <KPublicTransport/Manager>

namespace KPublicTransport {
class Journey;
class JourneySection;
class RentalVehicle;
class Stopover;
class StopoverRequest;
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

    /** Same as the above, but usable from QML. */
    Q_INVOKABLE QVariant trainStationFromLocation(const KPublicTransport::Location &loc) const;

    /** Applies information from @p location to the given KItinerary place.
     *  Unlike mergeStation(), this does not assume both sides refer to the same location, and @p location has precedence.
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

    /** Update a reservation from a KPublicTransport::Stopover.
     *  This assumes the stopover matches the departure/arrival of the reservation, and in case
     *  of conflict, the reservation is preferred.
     */
    static QVariant mergeDeparture(const QVariant &res, const KPublicTransport::Stopover &dep);
    static QVariant mergeArrival(const QVariant &res, const KPublicTransport::Stopover &arr);
    static QVariant mergeJourney(const QVariant &res, const KPublicTransport::JourneySection &journey);

    /** Checks if the given reservation and journey section have a compatible mode of transportation. */
    static bool isSameMode(const QVariant &res, KPublicTransport::Line::Mode mode);
    static bool isSameMode(const QVariant &res, const KPublicTransport::JourneySection &section);

    /** Provide an icon source for usage in Kirigami.Icon that represents the line mode.
     *  This can be an official line logo, and official product logo or a generic mode icon,
     *  depending on what's available.
     *  @note line/product logos are full-color, the generic ones are monochrome colorable Breeze icons.
     */
    Q_INVOKABLE QString lineIcon(const QVariant &line) const;

    /** Convert a KPublicTransport::Line::Mode enum value to an icon source or name
     *  for usage in Kirigami.Icon.
     */
    Q_INVOKABLE QString lineModeIcon(int lineMode) const;

    /** Provides an icon for a rental vehicle type. */
    Q_INVOKABLE QString rentalVehicleIcon(const KPublicTransport::RentalVehicle &vehicle) const;

    /** Provide an icon source for usage in Kirigami.Icon that represents @p journeySection. */
    Q_INVOKABLE QString journeySectionIcon(const KPublicTransport::JourneySection &journeySection) const;

    /** Create a KPublicTransport::StopoverRequest for the given KItinerary place. */
    Q_INVOKABLE KPublicTransport::StopoverRequest stopoverRequestForPlace(const QVariant &place, const QDateTime &dt) const;

    /** Produces a short summary of the given attribution information. */
    Q_INVOKABLE QString attributionSummary(const QVariantList &attributions) const;

    /** Returns @c true when we want to highlight @p sections due to looking problematic. */
    Q_INVOKABLE bool warnAboutSection(const KPublicTransport::JourneySection &section) const;

    /** First public transport section of the given section. */
    static KPublicTransport::JourneySection firstTransportSection(const KPublicTransport::Journey &journey);
    /** Last public transport section of the given section. */
    static KPublicTransport::JourneySection lastTransportSection(const KPublicTransport::Journey &journey);


    /** Selects suitable backends based on UIC operator codes from the reservation. */
    template <typename Request>
    static void selectBackends(Request &request, KPublicTransport::Manager *mgr, const QVariant &res);
private:
    // for use by the template code
    static QString idenfitierFromLocation(const KPublicTransport::Location &loc);
    static KItinerary::PostalAddress addressFromLocation(const KPublicTransport::Location &loc);
    static QStringList suitableBackendsForReservation(KPublicTransport::Manager *mgr, const QVariant &res);
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

template <typename Request>
inline void PublicTransport::selectBackends(Request &request, KPublicTransport::Manager *mgr, const QVariant &res)
{
    request.setBackendIds(suitableBackendsForReservation(mgr, res));
}

#endif // PUBLICTRANSPORT_H
