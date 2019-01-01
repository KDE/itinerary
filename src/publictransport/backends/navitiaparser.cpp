/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

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

#include "navitiaparser.h"

#include <KPublicTransport/Departure>
#include <KPublicTransport/Journey>
#include <KPublicTransport/Line>

#include <QColor>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimeZone>

using namespace KPublicTransport;

static QDateTime parseDateTime(const QJsonValue &v, const QTimeZone &tz)
{
    auto dt = QDateTime::fromString(v.toString(), QStringLiteral("yyyyMMddTHHmmss"));
    if (tz.isValid()) {
        dt.setTimeZone(tz);
    }
    return dt;
}

struct {
    const char *name;
    Line::Mode mode;
} static const navitia_phyiscal_modes[] = {
    { "Air", Line::Air },
    { "Boat", Line::Boat },
    { "Bus", Line::Bus },
    { "BusRapidTransit", Line::BusRapidTransit },
    { "Coach", Line::Coach },
    { "Ferry", Line::Ferry },
    { "Funicular", Line::Funicular },
    { "LocalTrain", Line::LocalTrain },
    { "LongDistanceTrain", Line::LongDistanceTrain },
    { "Metro", Line::Metro },
    { "RailShuttle", Line::RailShuttle },
    { "RapidTransit", Line::RapidTransit },
    { "Shuttle", Line::Shuttle },
    { "Taxi", Line::Taxi },
    { "Train", Line::Train },
    { "Tramway", Line::Tramway }
};

static Line::Mode parsePhysicalMode(const QString &mode)
{
    const auto modeStr = mode.toLatin1();
    if (!modeStr.startsWith("physical_mode:")) {
        return Line::Unknown;
    }
    for (auto it = std::begin(navitia_phyiscal_modes); it != std::end(navitia_phyiscal_modes); ++it) {
        if (strcmp(modeStr.constData() + 14, it->name) == 0) {
            return it->mode;
        }
    }

    return Line::Unknown;
}

static Location parseLocation(const QJsonObject &obj)
{
    Location loc;
    loc.setName(obj.value(QLatin1String("label")).toString());
    // TODO parse more fields

    const auto coord = obj.value(QLatin1String("coord")).toObject();
    loc.setCoordinate(coord.value(QLatin1String("lat")).toString().toDouble(), coord.value(QLatin1String("lon")).toString().toDouble());

    auto tz = obj.value(QLatin1String("timezone")).toString();
    if (tz.isEmpty()) {
        tz = obj.value(QLatin1String("stop_area")).toObject().value(QLatin1String("timezone")).toString();
    }
    if (!tz.isEmpty()) {
        loc.setTimeZone(QTimeZone(tz.toUtf8()));
    }

    return loc;
}

static Location parseWrappedLocation(const QJsonObject &obj)
{
    auto loc = parseLocation(obj.value(obj.value(QLatin1String("embedded_type")).toString()).toObject());
    loc.setName(obj.value(QLatin1String("name")).toString());
    return loc;
}

static JourneySection parseJourneySection(const QJsonObject &obj)
{
    const auto displayInfo = obj.value(QLatin1String("display_informations")).toObject();

    Line line;
    line.setName(displayInfo.value(QLatin1String("label")).toString());
    line.setColor(QColor(QLatin1Char('#') + displayInfo.value(QLatin1String("color")).toString()));
    line.setTextColor(QColor(QLatin1Char('#') + displayInfo.value(QLatin1String("text_color")).toString()));
    line.setModeString(displayInfo.value(QLatin1String("commercial_mode")).toString());
    const auto links = obj.value(QLatin1String("links")).toArray();
    for (const auto &v : links) {
        const auto link = v.toObject();
        if (link.value(QLatin1String("type")).toString() != QLatin1String("physical_mode")) {
            continue;
        }
        line.setMode(parsePhysicalMode(link.value(QLatin1String("id")).toString()));
        break;
    }

    Route route;
    route.setDirection(displayInfo.value(QLatin1String("direction")).toString());
    route.setLine(line);

    JourneySection section;
    section.setFrom(parseWrappedLocation(obj.value(QLatin1String("from")).toObject()));
    section.setTo(parseWrappedLocation(obj.value(QLatin1String("to")).toObject()));
    section.setRoute(route);
    section.setDepartureTime(parseDateTime(obj.value(QLatin1String("departure_date_time")), section.from().timeZone()));
    section.setArrivalTime(parseDateTime(obj.value(QLatin1String("arrival_date_time")), section.to().timeZone()));

    const auto typeStr = obj.value(QLatin1String("type")).toString();
    if (typeStr == QLatin1String("public_transport")) {
        section.setMode(JourneySection::PublicTransport);
    } else if (typeStr == QLatin1String("transfer")) {
        section.setMode(JourneySection::Transfer);
    } else if (typeStr == QLatin1String("street_network")) {
        section.setMode(JourneySection::Walking);
    } else if (typeStr == QLatin1String("waiting")) {
        section.setMode(JourneySection::Waiting);
    }

    return section;
}

static Journey parseJourney(const QJsonObject &obj)
{
    Journey journey;

    const auto secArray = obj.value(QLatin1String("sections")).toArray();
    std::vector<JourneySection> sections;
    sections.reserve(secArray.size());
    for (const auto &v : secArray) {
        sections.push_back(parseJourneySection(v.toObject()));
    }
    journey.setSections(std::move(sections));
    return journey;
}

std::vector<Journey> NavitiaParser::parseJourneys(const QByteArray &data)
{
    const auto topObj = QJsonDocument::fromJson(data).object();
    const auto journeys = topObj.value(QLatin1String("journeys")).toArray();

    std::vector<Journey> res;
    res.reserve(journeys.size());

    for (const auto &v : journeys) {
        res.push_back(parseJourney(v.toObject()));
    }

    return res;
}

static Departure parseDeparture(const QJsonObject &obj)
{
    // TODO remove code duplication with journey parsing
    const auto displayInfo = obj.value(QLatin1String("display_informations")).toObject();

    Line line;
    line.setName(displayInfo.value(QLatin1String("label")).toString());
    line.setColor(QColor(QLatin1Char('#') + displayInfo.value(QLatin1String("color")).toString()));
    line.setTextColor(QColor(QLatin1Char('#') + displayInfo.value(QLatin1String("text_color")).toString()));
    line.setModeString(displayInfo.value(QLatin1String("commercial_mode")).toString());
    const auto links = obj.value(QLatin1String("links")).toArray();
    for (const auto &v : links) {
        const auto link = v.toObject();
        if (link.value(QLatin1String("type")).toString() != QLatin1String("physical_mode")) {
            continue;
        }
        line.setMode(parsePhysicalMode(link.value(QLatin1String("id")).toString()));
        break;
    }

    Route route;
    route.setDirection(displayInfo.value(QLatin1String("direction")).toString());
    route.setLine(line);

    Departure departure;
    departure.setRoute(route);

    const auto dtObj = obj.value(QLatin1String("stop_date_time")).toObject();
    departure.setStopPoint(parseLocation(obj.value(QLatin1String("stop_point")).toObject()));
    departure.setScheduledDepartureTime(parseDateTime(dtObj.value(QLatin1String("base_departure_date_time")), departure.stopPoint().timeZone()));
    departure.setScheduledArrivalTime(parseDateTime(dtObj.value(QLatin1String("base_arrival_date_time")), departure.stopPoint().timeZone()));
    if (dtObj.value(QLatin1String("data_freshness")).toString() != QLatin1String("base_schedule")) {
        departure.setExpectedDepartureTime(parseDateTime(dtObj.value(QLatin1String("departure_date_time")), departure.stopPoint().timeZone()));
        departure.setExpectedArrivalTime(parseDateTime(dtObj.value(QLatin1String("arrival_date_time")), departure.stopPoint().timeZone()));
    }

    return departure;
}

std::vector<Departure> NavitiaParser::parseDepartures(const QByteArray &data)
{
    const auto topObj = QJsonDocument::fromJson(data).object();
    const auto departures = topObj.value(QLatin1String("departures")).toArray();

    std::vector<Departure> res;
    res.reserve(departures.size());

    for (const auto &v : departures) {
        res.push_back(parseDeparture(v.toObject()));
    }

    return res;
}

std::vector<Location> NavitiaParser::parsePlacesNearby(const QByteArray &data)
{
    const auto topObj = QJsonDocument::fromJson(data).object();
    const auto placesNearby = topObj.value(QLatin1String("places_nearby")).toArray();

    std::vector<Location> res;
    res.reserve(placesNearby.size());

    for (const auto &v : placesNearby) {
        res.push_back(parseWrappedLocation(v.toObject()));
    }

    return res;
}

std::vector<Location> NavitiaParser::parsePlaces(const QByteArray &data)
{
    const auto topObj = QJsonDocument::fromJson(data).object();
    const auto placesNearby = topObj.value(QLatin1String("places")).toArray();

    std::vector<Location> res;
    res.reserve(placesNearby.size());

    for (const auto &v : placesNearby) {
        res.push_back(parseWrappedLocation(v.toObject()));
    }

    return res;
}

QString NavitiaParser::parseErrorMessage(const QByteArray &data)
{
    const auto topObj = QJsonDocument::fromJson(data).object();
    const auto errorObj = topObj.value(QLatin1String("error")).toObject();

    // id field contains error enum, might also be useful
    return errorObj.value(QLatin1String("message")).toString();
}
