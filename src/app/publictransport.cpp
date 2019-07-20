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
#include "logging.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/MergeUtil>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/Attribution>
#include <KPublicTransport/DepartureRequest>
#include <KPublicTransport/Journey>
#include <KPublicTransport/Line>
#include <KPublicTransport/Location>

#include <QDateTime>
#include <QDebug>
#include <QUrl>

KPublicTransport::Location PublicTransport::locationFromPlace(const QVariant& place)
{
    KPublicTransport::Location loc;
    loc.setName(KItinerary::LocationUtil::name(place));
    const auto geo = KItinerary::LocationUtil::geo(place);
    loc.setCoordinate(geo.latitude(), geo.longitude());

    if (KItinerary::JsonLd::canConvert<KItinerary::Place>(place)) {
        const auto p = KItinerary::JsonLd::convert<KItinerary::Place>(place);
        if (!p.identifier().isEmpty()) {
            const auto idSplit = p.identifier().split(QLatin1Char(':'));
            if (idSplit.size() == 2) {
                loc.setIdentifier(idSplit.at(0), idSplit.at(1));
            }
        }
    }

    return loc;
}

QString PublicTransport::lineModeIcon(int lineMode)
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

KItinerary::TrainStation PublicTransport::mergeStation(const KItinerary::TrainStation &station, const KPublicTransport::Location &loc)
{
    using namespace KItinerary;
    return MergeUtil::merge(placeFromLocation<TrainStation>(loc), station).value<TrainStation>();
}

KItinerary::TrainStation PublicTransport::applyStation(const KItinerary::TrainStation &station, const KPublicTransport::Location &loc)
{
    using namespace KItinerary;

    auto newStation = placeFromLocation<TrainStation>(loc);
    if (LocationUtil::isSameLocation(station, newStation)) {
        return MergeUtil::merge(station, newStation).value<TrainStation>();
    }

    return newStation;
}

static KItinerary::TrainReservation applyJourneySection(KItinerary::TrainReservation res, const KPublicTransport::JourneySection &section)
{
    auto trip = res.reservationFor().value<KItinerary::TrainTrip>();
    trip.setDepartureTime(section.scheduledDepartureTime());
    trip.setArrivalTime(section.scheduledArrivalTime());
    trip.setTrainNumber(section.route().line().name());
    trip.setTrainName(section.route().line().modeString());
    trip.setDeparturePlatform(section.scheduledDeparturePlatform());
    trip.setArrivalPlatform(section.scheduledArrivalPlatform());

    trip.setDepartureStation(PublicTransport::applyStation(trip.departureStation(), section.from()));
    trip.setArrivalStation(PublicTransport::applyStation(trip.arrivalStation(), section.to()));

    res.setReservationFor(trip);
    return res;
}

QVariant PublicTransport::applyJourneySection(const QVariant &res, const KPublicTransport::JourneySection &section)
{
    using namespace KItinerary;

    if (JsonLd::isA<TrainReservation>(res)) {
        return ::applyJourneySection(res.value<TrainReservation>(), section);
    }

    qWarning() << res.typeName() << "NOT IMPLEMENTED YET!";
    return res;
}

bool PublicTransport::isSameMode(const QVariant &res, const KPublicTransport::JourneySection &section)
{
    using namespace KPublicTransport;

    if (KItinerary::JsonLd::isA<KItinerary::TrainReservation>(res)) {
        switch (section.route().line().mode()) {
            case Line::Train:
            case Line::Funicular:
            case Line::LocalTrain:
            case Line::LongDistanceTrain:
            case Line::Metro:
            case Line::RailShuttle:
            case Line::RapidTransit:
            case Line::Tramway:
                return true;
            default:
                return false;
        }
    } else if (KItinerary::JsonLd::isA<KItinerary::BusReservation>(res)) {
        switch (section.route().line().mode()) {
            case Line::Bus:
            case Line::BusRapidTransit:
            case Line::Coach:
                return true;
            default:
                return false;
        }
    } else if (res.isValid()) {
        qCWarning(Log) << "unexpected reservation type!?" << res;
    }

    return false;
}


QVariant PublicTransport::departureRequestForPlace(const QVariant &place, const QDateTime &dt) const
{
    KPublicTransport::DepartureRequest req;
    req.setDateTime(std::max(dt, QDateTime::currentDateTime()));
    req.setStop(PublicTransport::locationFromPlace(place));
    return QVariant::fromValue(req);
}

QString PublicTransport::attributionSummary(const QVariantList& attributions) const
{
    QStringList l;
    l.reserve(attributions.size());
    for (const auto &v : attributions) {
        const auto attr = v.value<KPublicTransport::Attribution>();
        l.push_back(QLatin1String("<a href=\"") + attr.url().toString() + QLatin1String("\">") + attr.name() + QLatin1String("</a>"));
    }
    return l.join(QLatin1String(", "));
}

QString PublicTransport::idenfitierFromLocation(const KPublicTransport::Location &loc)
{
    auto id = loc.identifier(QLatin1String("ibnr"));
    if (!id.isEmpty()) {
        return QLatin1String("ibnr:") + id;
    }
    id = loc.identifier(QLatin1String("uic"));
    if (!id.isEmpty()) {
        return QLatin1String("uic:") + id;
    }
    return {};
}

KItinerary::PostalAddress PublicTransport::addressFromLocation(const KPublicTransport::Location &loc)
{
    KItinerary::PostalAddress addr;
    addr.setStreetAddress(loc.streetAddress());
    addr.setPostalCode(loc.postalCode());
    addr.setAddressLocality(loc.locality());
    addr.setAddressRegion(loc.region());
    addr.setAddressCountry(loc.country());
    return addr;
}

#include "moc_publictransport.cpp"
