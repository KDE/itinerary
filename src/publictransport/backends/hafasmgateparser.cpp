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
#include <KPublicTransport/Journey>
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

void HafasMgateParser::setLocationIdentifierType(const QString &idType)
{
    m_locIdType = idType;
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

std::vector<Location> HafasMgateParser::parseLocations(const QJsonArray &locL) const
{
    std::vector<Location> locs;
    locs.reserve(locL.size());
    for (const auto &locV : locL) {
        const auto locObj = locV.toObject();
        Location loc;
        loc.setName(locObj.value(QLatin1String("name")).toString());
        loc.setIdentifier(m_locIdType, locObj.value(QLatin1String("extId")).toString());
        const auto coordObj = locObj.value(QLatin1String("crd")).toObject();
        loc.setCoordinate(coordObj.value(QLatin1String("y")).toDouble() / 1000000.0, coordObj.value(QLatin1String("x")).toDouble() / 1000000.0);
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
        dep.setScheduledDepartureTime(parseDateTime(dateStr, stbStop.value(QLatin1String("dTimeS")).toString()));
        dep.setExpectedDepartureTime(parseDateTime(dateStr, stbStop.value(QLatin1String("dTimeR")).toString()));
        dep.setScheduledArrivalTime(parseDateTime(dateStr, stbStop.value(QLatin1String("aTimeS")).toString()));
        dep.setExpectedArrivalTime(parseDateTime(dateStr, stbStop.value(QLatin1String("aTimeR")).toString()));

        dep.setScheduledPlatform(stbStop.value(QLatin1String("dPlatfS")).toString());
        dep.setExpectedPlatform(stbStop.value(QLatin1String("dPlatfR")).toString());
        if (dep.scheduledPlatform().isEmpty()) {
            dep.setScheduledPlatform(stbStop.value(QLatin1String("aPlatfS")).toString());
        }
        if (dep.expectedPlatform().isEmpty()) {
            dep.setExpectedPlatform(stbStop.value(QLatin1String("aPlatfR")).toString());
        }

        const auto locIdx = stbStop.value(QLatin1String("locX")).toInt();
        if ((unsigned int)locIdx < locs.size()) {
            dep.setStopPoint(locs[locIdx]);
        }

        res.push_back(dep);
    }

    return res;
}

bool HafasMgateParser::parseError(const QJsonObject& obj) const
{
    if (obj.value(QLatin1String("err")).toString() != QLatin1String("OK")) {
        m_error = Reply::NotFoundError;
        m_errorMsg = obj.value(QLatin1String("errTxt")).toString();
        return false;
    }

    m_error = Reply::NoError;
    m_errorMsg.clear();
    return true;
}


std::vector<Departure> HafasMgateParser::parseDepartures(const QByteArray &data) const
{
    const auto topObj = QJsonDocument::fromJson(data).object();
    //qDebug().noquote() << QJsonDocument(topObj).toJson();
    const auto svcResL = topObj.value(QLatin1String("svcResL")).toArray();

    for (const auto &v : svcResL) {
        const auto obj = v.toObject();
        if (obj.value(QLatin1String("meth")).toString() == QLatin1String("StationBoard")) {
            if (parseError(obj)) {
                return parseStationBoardResponse(obj.value(QLatin1String("res")).toObject());
            }
            return {};
        }
    }

    return {};
}

std::vector<Location> HafasMgateParser::parseLocations(const QByteArray &data) const
{
    const auto topObj = QJsonDocument::fromJson(data).object();
    const auto svcResL = topObj.value(QLatin1String("svcResL")).toArray();

    for (const auto &v : svcResL) {
        const auto obj = v.toObject();
        const auto meth = obj.value(QLatin1String("meth")).toString();
        if (meth == QLatin1String("LocMatch") || meth == QLatin1String("LocGeoPos")) {
            if (parseError(obj)) {
                const auto resObj = obj.value(QLatin1String("res")).toObject();
                if (resObj.contains(QLatin1String("locL"))) {
                    return parseLocations(resObj.value(QLatin1String("locL")).toArray());
                }
                if (resObj.contains(QLatin1String("match"))) {
                    return parseLocations(resObj.value(QLatin1String("match")).toObject().value(QLatin1String("locL")).toArray());
                }
                qCDebug(Log).noquote() << "Failed to parse location query response:" << QJsonDocument(obj).toJson();
                return {};
            }
            return {};
        }
    }

    return {};
}

std::vector<Journey> HafasMgateParser::parseJourneys(const QByteArray &data) const
{
    const auto topObj = QJsonDocument::fromJson(data).object();
//     qDebug().noquote() << QJsonDocument(topObj).toJson();
    const auto svcResL = topObj.value(QLatin1String("svcResL")).toArray();

    for (const auto &v : svcResL) {
        const auto obj = v.toObject();
        if (obj.value(QLatin1String("meth")).toString() == QLatin1String("TripSearch")) {
            if (parseError(obj)) {
                return parseTripSearch(obj.value(QLatin1String("res")).toObject());
            }
            return {};
        }
    }

    return {};
}

std::vector<Journey> HafasMgateParser::parseTripSearch(const QJsonObject &obj) const
{
    const auto commonObj = obj.value(QLatin1String("common")).toObject();
    const auto icos = parseIcos(commonObj.value(QLatin1String("icoL")).toArray());
    const auto locs = parseLocations(commonObj.value(QLatin1String("locL")).toArray());
    const auto lines = parseLines(commonObj.value(QLatin1String("prodL")).toArray(), icos);

    std::vector<Journey> res;
    const auto outConL = obj.value(QLatin1String("outConL")).toArray();
    res.reserve(outConL.size());

    for (const auto &outConV: outConL) {
        const auto outCon = outConV.toObject();

        const auto dateStr = outCon.value(QLatin1String("date")).toString();

        const auto secL = outCon.value(QLatin1String("secL")).toArray();
        std::vector<JourneySection> sections;
        sections.reserve(secL.size());


        for (const auto &secV : secL) {
            const auto secObj = secV.toObject();
            JourneySection section;

            const auto dep = secObj.value(QLatin1String("dep")).toObject();
            section.setScheduledDepartureTime(parseDateTime(dateStr, dep.value(QLatin1String("dTimeS")).toString()));
            section.setExpectedDepartureTime(parseDateTime(dateStr, dep.value(QLatin1String("dTimeR")).toString()));
            auto locIdx = dep.value(QLatin1String("locX")).toInt();
            if ((unsigned int)locIdx < locs.size()) {
                section.setFrom(locs[locIdx]);
            }
            section.setScheduledDeparturePlatform(dep.value(QLatin1String("dPlatfS")).toString());
            section.setExpectedDeparturePlatform(dep.value(QLatin1String("dPlatfR")).toString());

            const auto arr = secObj.value(QLatin1String("arr")).toObject();
            section.setScheduledArrivalTime(parseDateTime(dateStr, arr.value(QLatin1String("aTimeS")).toString()));
            section.setExpectedArrivalTime(parseDateTime(dateStr, arr.value(QLatin1String("aTimeR")).toString()));
            locIdx = arr.value(QLatin1String("locX")).toInt();
            if ((unsigned int)locIdx < locs.size()) {
                section.setTo(locs[locIdx]);
            }
            section.setScheduledArrivalPlatform(dep.value(QLatin1String("aPlatfS")).toString());
            section.setExpectedArrivalPlatform(dep.value(QLatin1String("aPlatfR")).toString());

            const auto typeStr = secObj.value(QLatin1String("type")).toString();
            if (typeStr == QLatin1String("JNY")) {
                section.setMode(JourneySection::PublicTransport);

                const auto jnyObj = secObj.value(QLatin1String("jny")).toObject();
                Route route;
                route.setDirection(jnyObj.value(QLatin1String("dirTxt")).toString());
                const auto lineIdx = jnyObj.value(QLatin1String("prodX")).toInt();
                if ((unsigned int)lineIdx < lines.size()) {
                    route.setLine(lines[lineIdx]);
                }
                section.setRoute(route);
            } else if (typeStr == QLatin1String("WALK")) {
                section.setMode(JourneySection::Walking);
            } else if (typeStr == QLatin1String("TRSF")) {
                section.setMode(JourneySection::Transfer);
            }

            sections.push_back(section);
        }

        Journey journey;
        journey.setSections(std::move(sections));
        res.push_back(journey);
    }

    return res;
}

Reply::Error HafasMgateParser::error() const
{
    return m_error;
}

QString HafasMgateParser::errorMessage() const
{
    return m_errorMsg;
}

QDateTime HafasMgateParser::parseDateTime(const QString &date, const QString &time)
{
    if (date.isEmpty() || time.isEmpty()) {
        return {};
    }

    int dayOffset = 0;
    if (time.size() > 6) {
        dayOffset = time.leftRef(time.size() - 6).toInt();
    }

    const auto dt = QDateTime::fromString(date + time.rightRef(6), QLatin1String("yyyyMMddhhmmss"));
    return dt.addDays(dayOffset);
}
