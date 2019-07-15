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

#include "publictransport.h"

#include <KItinerary/Place>

#include <KPublicTransport/Line>
#include <KPublicTransport/Location>

KPublicTransport::Location PublicTransport::locationFromStation(const KItinerary::TrainStation& station)
{
    using namespace KPublicTransport;
    Location loc;
    loc.setName(station.name());
    loc.setCoordinate(station.geo().latitude(), station.geo().longitude());
    if (!station.identifier().isEmpty()) {
        const auto idSplit = station.identifier().split(QLatin1Char(':'));
        if (idSplit.size() == 2) {
            loc.setIdentifier(idSplit.at(0), idSplit.at(1));
        }
    }
    return loc;
}

KPublicTransport::Location PublicTransport::locationFromStation(const KItinerary::BusStation &busStop)
{
    using namespace KPublicTransport;
    Location loc;
    loc.setName(busStop.name());
    loc.setCoordinate(busStop.geo().latitude(), busStop.geo().longitude());
    return loc;
}

QString PublicTransportUtil::lineModeIcon(int lineMode)
{
    using namespace KPublicTransport;
    switch (lineMode) {
        case Line::Air:
            return QStringLiteral("qrc:///images/flight.svg");
        case Line::Boat:
        case Line::Ferry:
            return QStringLiteral("qrc:///images/ferry.svg");
        case Line::Bus:
            return QStringLiteral("qrc:///images/bus.svg");
        case Line::BusRapidTransit:
        case Line::Coach:
            return QStringLiteral("qrc:///images/coach.svg");
        case Line::Funicular:
            return QStringLiteral("qrc:///images/Funicular.svg");
        case Line::LocalTrain:
        case Line::Train:
            return QStringLiteral("qrc:///images/train.svg");
        case Line::LongDistanceTrain:
            return QStringLiteral("qrc:///images/longdistancetrain.svg");
        case Line::Metro:
            return QStringLiteral("qrc:///images/subway.svg");
        case Line::RailShuttle:
        case Line::RapidTransit:
            return QStringLiteral("qrc:///images/rapidtransit.svg");
        case Line::Shuttle:
            return QStringLiteral("qrc:///images/shuttle.svg");
        case Line::Taxi:
            return QStringLiteral("qrc:///images/taxi.svg");
        case Line::Tramway:
            return QStringLiteral("qrc:///images/tramway.svg");
    }

    return QStringLiteral("question");
}

KItinerary::TrainStation PublicTransport::mergeStation(KItinerary::TrainStation station, const KPublicTransport::Location &loc)
{
    if (!station.geo().isValid() && loc.hasCoordinate()) {
        station.setGeo(KItinerary::GeoCoordinates{loc.latitude(), loc.longitude()});
    }

    auto addr = station.address();
    if (addr.streetAddress().isEmpty() && !loc.streetAddress().isEmpty()) {
        addr.setStreetAddress(loc.locality());
    }
    if (addr.postalCode().isEmpty() && !loc.postalCode().isEmpty()) {
        addr.setPostalCode(loc.locality());
    }
    if (addr.addressLocality().isEmpty() && !loc.locality().isEmpty()) {
        addr.setAddressLocality(loc.locality());
    }
    if (addr.addressRegion().isEmpty() && !loc.region().isEmpty()) {
        addr.setAddressRegion(loc.locality());
    }
    if (addr.addressCountry().isEmpty() && !loc.country().isEmpty()) {
        addr.setAddressCountry(loc.locality());
    }
    station.setAddress(addr);

    // TODO ibnr/uic station ids
    // for this to take full effect we might need a pass through the KItinerary post-processor

    return station;
}
