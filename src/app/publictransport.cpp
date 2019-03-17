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
