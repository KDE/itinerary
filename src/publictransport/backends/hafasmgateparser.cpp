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

#include "hafasmgateparser.h"

#include <KPublicTransport/Departure>
#include <KPublicTransport/Line>

#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace KPublicTransport;

static std::vector<Location> parseLocations(const QJsonArray &locL)
{
    std::vector<Location> locs;
    locs.reserve(locL.size());
    for (const auto &locV : locL) {
        const auto locObj = locV.toObject();
        Location loc;
        loc.setName(locObj.value(QLatin1String("name")).toString());
        // TODO extId
        const auto coordObj = locObj.value(QLatin1String("crd")).toObject();
        loc.setCoordinate(coordObj.value(QLatin1String("x")).toDouble() / 1000000.0, coordObj.value(QLatin1String("y")).toDouble() / 1000000.0);
        locs.push_back(loc);
    }
    return locs;
}

// ### mapping based on SNCB, is that the same everywhere? if not, this needs to move to the network config
static struct {
    int productCode;
    Line::Mode mode;
} const line_type_map[] {
    { 4, Line::Train },
    { 64, Line::RapidTransit },
    { 256, Line::Metro },
    { 512, Line::Bus },
    { 1024, Line::Tramway }
};

static std::vector<Line> parseLines(const QJsonArray &prodL)
{
    std::vector<Line> lines;
    lines.reserve(prodL.size());
    for (const auto &prodV : prodL) {
        const auto prodObj = prodV.toObject();
        Line line;
        line.setName(prodObj.value(QLatin1String("name")).toString());

        const auto prodCode = prodObj.value(QLatin1String("cls")).toInt();
        for (auto it = std::begin(line_type_map); it != std::end(line_type_map); ++it) {
            if ((*it).productCode == prodCode) {
                line.setMode((*it).mode);
                break;
            }
        }
        lines.push_back(line);
    }

    return lines;
}

static std::vector<Departure> parseStationBoardResponse(const QJsonObject &obj)
{
    const auto commonObj = obj.value(QLatin1String("common")).toObject();
    const auto locs = parseLocations(commonObj.value(QLatin1String("locL")).toArray());
    const auto lines = parseLines(commonObj.value(QLatin1String("prodL")).toArray());

    std::vector<Departure> res;
    const auto jnyL = obj.value(QLatin1String("jnyL")).toArray();
    res.reserve(jnyL.size());

    for (const auto &jny : jnyL) {
        const auto jnyObj = jny.toObject();
        const auto stbStop = jnyObj.value(QLatin1String("stbStop")).toObject();

        Route route;
        route.setDirection(jnyObj.value(QLatin1String("dirTxt")).toString());
        const auto lineIdx = stbStop.value(QLatin1String("prodX")).toInt();
        if ((unsigned int)lineIdx < lines.size()) {
            route.setLine(lines[lineIdx]);
        }

        Departure dep;
        dep.setRoute(route);

        const auto dateStr = jnyObj.value(QLatin1String("date")).toString();
        dep.setScheduledTime(QDateTime::fromString(dateStr + stbStop.value(QLatin1String("dTimeS")).toString(), QLatin1String("yyyyMMddhhmmss")));
        const auto dTimeP = stbStop.value(QLatin1String("dTimeP")).toString();
        if (!dTimeP.isEmpty()) {
            dep.setActualTime(QDateTime::fromString(dateStr + dTimeP, QLatin1String("yyyyMMddhhmmss")));
        }

        // TODO platform

        const auto locIdx = stbStop.value(QLatin1String("locX")).toInt();
        if ((unsigned int)locIdx < locs.size()) {
            dep.setStopPoint(locs[locIdx]);
        }

        res.push_back(dep);
    }

    return res;
}

std::vector<Departure> HafasMgateParser::parseDepartures(const QByteArray &data)
{
    const auto topObj = QJsonDocument::fromJson(data).object();
    const auto svcResL = topObj.value(QLatin1String("svcResL")).toArray();

    for (const auto &v : svcResL) {
        const auto obj = v.toObject();
        if (obj.value(QLatin1String("meth")).toString() == QLatin1String("StationBoard")) {
            return parseStationBoardResponse(obj.value(QLatin1String("res")).toObject());
        }
    }

    return {};
}
