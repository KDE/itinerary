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
#include "logging.h"

#include <KPublicTransport/Departure>
#include <KPublicTransport/Line>

#include <QColor>
#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace KPublicTransport;

HafasMgateParser::HafasMgateParser() = default;
HafasMgateParser::~HafasMgateParser() = default;

void HafasMgateParser::setLineModeMap(std::unordered_map<int, Line::Mode> &&modeMap)
{
    m_lineModeMap = std::move(modeMap);
}

static std::vector<Ico> parseIcos(const QJsonArray &icoL)
{
    std::vector<Ico> icos;
    icos.reserve(icoL.size());
    for (const auto &icoV : icoL) {
        const auto icoObj = icoV.toObject();
        Ico ico;
        const auto fg = icoObj.value(QLatin1String("fg")).toObject();
        if (!fg.isEmpty()) {
            ico.fg = QColor(fg.value(QLatin1String("r")).toInt(), fg.value(QLatin1String("g")).toInt(), fg.value(QLatin1String("b")).toInt());
        }
        const auto bg = icoObj.value(QLatin1String("bg")).toObject();
        if (!bg.isEmpty()) {
            ico.bg = QColor(bg.value(QLatin1String("r")).toInt(), bg.value(QLatin1String("g")).toInt(), bg.value(QLatin1String("b")).toInt());
        }
        icos.push_back(ico);
    }
    return icos;
}

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

std::vector<Line> HafasMgateParser::parseLines(const QJsonArray &prodL, const std::vector<Ico> &icos) const
{
    std::vector<Line> lines;
    lines.reserve(prodL.size());
    for (const auto &prodV : prodL) {
        const auto prodObj = prodV.toObject();
        Line line;
        line.setName(prodObj.value(QLatin1String("name")).toString());

        const auto prodCode = prodObj.value(QLatin1String("cls")).toInt();
        const auto lineModeIt = m_lineModeMap.find(prodCode);
        if (lineModeIt != m_lineModeMap.end()) {
            line.setMode((*lineModeIt).second);
        } else {
            qCDebug(Log) << "Encountered unknown line type:" << prodCode << line.name();
        }

        const auto icoIdx = prodObj.value(QLatin1String("icoX")).toInt();
        if ((unsigned int)icoIdx < icos.size()) {
            line.setColor(icos[icoIdx].bg);
            line.setTextColor(icos[icoIdx].fg);
        }

        lines.push_back(line);
    }

    return lines;
}

std::vector<Departure> HafasMgateParser::parseStationBoardResponse(const QJsonObject &obj) const
{
    const auto commonObj = obj.value(QLatin1String("common")).toObject();
    const auto icos = parseIcos(commonObj.value(QLatin1String("icoL")).toArray());
    const auto locs = parseLocations(commonObj.value(QLatin1String("locL")).toArray());
    const auto lines = parseLines(commonObj.value(QLatin1String("prodL")).toArray(), icos);

    std::vector<Departure> res;
    const auto jnyL = obj.value(QLatin1String("jnyL")).toArray();
    res.reserve(jnyL.size());

    for (const auto &jny : jnyL) {
        const auto jnyObj = jny.toObject();
        const auto stbStop = jnyObj.value(QLatin1String("stbStop")).toObject();

        Route route;
        route.setDirection(jnyObj.value(QLatin1String("dirTxt")).toString());
        const auto lineIdx = jnyObj.value(QLatin1String("prodX")).toInt();
        if ((unsigned int)lineIdx < lines.size()) {
            route.setLine(lines[lineIdx]);
        }

        Departure dep;
        dep.setRoute(route);

        const auto dateStr = jnyObj.value(QLatin1String("date")).toString();
        dep.setScheduledTime(QDateTime::fromString(dateStr + stbStop.value(QLatin1String("dTimeS")).toString(), QLatin1String("yyyyMMddhhmmss")));
        const auto dTimeR = stbStop.value(QLatin1String("dTimeR")).toString();
        if (!dTimeR.isEmpty()) {
            dep.setExpectedTime(QDateTime::fromString(dateStr + dTimeR, QLatin1String("yyyyMMddhhmmss")));
        }

        dep.setScheduledPlatform(stbStop.value(QLatin1String("dPlatfS")).toString());
        dep.setExpectedPlatform(stbStop.value(QLatin1String("dPlatfR")).toString());

        const auto locIdx = stbStop.value(QLatin1String("locX")).toInt();
        if ((unsigned int)locIdx < locs.size()) {
            dep.setStopPoint(locs[locIdx]);
        }

        res.push_back(dep);
    }

    return res;
}

std::vector<Departure> HafasMgateParser::parseDepartures(const QByteArray &data) const
{
    const auto topObj = QJsonDocument::fromJson(data).object();
//     qDebug().noquote() << QJsonDocument(topObj).toJson();
    const auto svcResL = topObj.value(QLatin1String("svcResL")).toArray();

    for (const auto &v : svcResL) {
        const auto obj = v.toObject();
        if (obj.value(QLatin1String("meth")).toString() == QLatin1String("StationBoard")) {
            return parseStationBoardResponse(obj.value(QLatin1String("res")).toObject());
        }
    }

    return {};
}
