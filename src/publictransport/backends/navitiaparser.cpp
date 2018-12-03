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

#include <KPublicTransport/Journey>
#include <KPublicTransport/Line>

#include <QColor>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace KPublicTransport;

struct {
    const char *name;
    Line::Mode mode;
} static const navitia_phyiscal_modes[] = {
    { "Air", Line::Air },
    { "Boat", Line::Boat },
    { "Bus", Line::Bus },
    { "BusRapidTransit", Line::RapidTransit },
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
    loc.setName(obj.value(QLatin1String("name")).toString());
    // TODO parse more fields

    const auto embObj = obj.value(obj.value(QLatin1String("embedded_type")).toString()).toObject();
    const auto coord = embObj.value(QLatin1String("coord")).toObject();
    loc.setCoordinate(coord.value(QLatin1String("lat")).toString().toDouble(), coord.value(QLatin1String("lon")).toString().toDouble());

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
    section.setRoute(route);
    section.setDepartureTime(QDateTime::fromString(obj.value(QLatin1String("departure_date_time")).toString(), QStringLiteral("yyyyMMddTHHmmss")));
    section.setArrivalTime(QDateTime::fromString(obj.value(QLatin1String("arrival_date_time")).toString(), QStringLiteral("yyyyMMddTHHmmss")));
    section.setFrom(parseLocation(obj.value(QLatin1String("from")).toObject()));
    section.setTo(parseLocation(obj.value(QLatin1String("to")).toObject()));

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
