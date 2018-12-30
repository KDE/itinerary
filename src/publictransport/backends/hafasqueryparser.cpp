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

#include "hafasqueryparser.h"

#include <KPublicTransport/Departure>

#include <QDateTime>
#include <QDebug>
#include <QXmlStreamReader>

using namespace KPublicTransport;

std::vector<Departure> HafasQueryParser::parseStationBoardResponse(const QByteArray &data, bool isArrival)
{
    qDebug().noquote() << data;
    std::vector<Departure> res;

    QXmlStreamReader reader;
    if (data.startsWith("<Journey")) { // SBB and RT don't reply with valid XML...
        reader.addData("<dummyRoot>");
        reader.addData(data);
        reader.addData("</dummyRoot>");
    } else {
        reader.addData(data);
    }

    Location stopPoint;
    while (!reader.atEnd()) {
        const auto token = reader.readNext();
        switch (token) {
            case QXmlStreamReader::StartElement:
                if (reader.name() == QLatin1String("St")) {
                    stopPoint.setName(reader.attributes().value(QLatin1String("name")).toString());
                    stopPoint.setIdentifier(QLatin1String("ibnr"), reader.attributes().value(QLatin1String("evaId")).toString());
                } else if (reader.name() == QLatin1String("Journey")) {
                    auto dt = QDateTime::fromString(reader.attributes().value(QLatin1String("fpDate")) + reader.attributes().value(QLatin1String("fpTime")), QLatin1String("dd.MM.yyhh:mm"));
                    if (dt.date().year() < 2000) {
                        dt = dt.addYears(100);
                    }
                    const auto delayStr = reader.attributes().value(QLatin1String("e_delay"));
                    const auto delaySecs = delayStr.toInt() * 60;
                    Departure dep;
                    if (isArrival) {
                        dep.setScheduledArrivalTime(dt);
                        if (!delayStr.isEmpty()) {
                            dep.setExpectedArrivalTime(dt.addSecs(delaySecs));
                        }
                    } else {
                        dep.setScheduledDepartureTime(dt);
                        if (!delayStr.isEmpty()) {
                            dep.setExpectedDepartureTime(dt.addSecs(delaySecs));
                        }
                    }
                    dep.setScheduledPlatform(reader.attributes().value(QLatin1String("platform")).toString());
                    dep.setExpectedPlatform(reader.attributes().value(QLatin1String("newpl")).toString());

                    Route route;
                    route.setDirection(reader.attributes().value(QLatin1String("targetLoc")).toString());
                    Line line;
                    line.setName(reader.attributes().value(QLatin1String("hafasname")).toString());
                    if (line.name().isEmpty()) {
                        const auto prod = reader.attributes().value(QLatin1String("prod"));
                        const auto idx = prod.indexOf(QLatin1Char('#'));
                        line.setName(prod.left(idx).toString().simplified());
                    }
                    // TODO line mode (class attribute, or second part of prod attribute)
                    route.setLine(line);
                    dep.setRoute(route);

                    dep.setStopPoint(stopPoint);
                    res.push_back(dep);
                }
                break;
            default:
                break;
        }
    }

    return res;
}
