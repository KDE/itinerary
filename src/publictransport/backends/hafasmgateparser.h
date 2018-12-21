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

#ifndef KPUBLICTRANSPORT_HAFASMGATEPARSER_H
#define KPUBLICTRANSPORT_HAFASMGATEPARSER_H

#include <KPublicTransport/Line>
#include <KPublicTransport/Reply>

#include <unordered_map>
#include <vector>

#include <QColor>

class QByteArray;

namespace KPublicTransport {

class Departure;
class Location;

struct Ico {
    QColor bg;
    QColor fg;
};

/** Hafas response parser. */
class HafasMgateParser
{
public:
    HafasMgateParser();
    ~HafasMgateParser();
    void setLineModeMap(std::unordered_map<int, Line::Mode> &&modeMap);
    void setLocationIdentifierType(const QString &idType);

    std::vector<Departure> parseDepartures(const QByteArray &data) const;
    std::vector<Location> parseLocations(const QByteArray &data) const;

    Reply::Error error() const;
    QString errorMessage() const;

private:
    Q_DISABLE_COPY(HafasMgateParser)
    std::vector<Departure> parseStationBoardResponse(const QJsonObject &obj) const;
    std::vector<Line> parseLines(const QJsonArray &prodL, const std::vector<Ico> &icos) const;
    std::vector<Location> parseLocations(const QJsonArray &locL) const;
    bool parseError(const QJsonObject &obj) const;

    std::unordered_map<int, Line::Mode> m_lineModeMap;
    QString m_locIdType;
    mutable QString m_errorMsg;
    mutable Reply::Error m_error = Reply::NoError;
};

}

#endif // KPUBLICTRANSPORT_HAFASMGATEPARSER_H
