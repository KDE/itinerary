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

#ifndef KPUBLICTRANSPORT_LINE_H
#define KPUBLICTRANSPORT_LINE_H

#include "datatypes.h"

namespace KPublicTransport {

class LinePrivate;

/** A public transport line. */
class Line
{
    KPUBLICTRANSPORT_GADGET(Line)
    /** Name of the line. */
    Q_PROPERTY(QString name READ name WRITE setName)
    /** Color of the line. */
    Q_PROPERTY(QColor color READ color WRITE setColor)
    /** Text color to use on top of the line color. */
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
    /** Type of transport. */
    Q_PROPERTY(Mode mode READ mode WRITE setMode)
    /** Human readable representation of the type of transport.
     *  This is not necessarily a simple 1:1 mapping from mode, but can contain
     *  e.g. a product name.
     */
    Q_PROPERTY(QString modeString READ modeString WRITE setModeString)

public:
    QString name() const;
    void setName(const QString &name);
    QColor color() const;
    void setColor(const QColor &color);
    QColor textColor() const;
    void setTextColor(const QColor &textColor);

    enum Mode { // ### direct copy from Navitia, we maybe can reduce that a bit
        Unknown,
        Air,
        Boat,
        Bus,
        BusRapidTransit,
        Coach,
        Ferry,
        Funicular,
        LocalTrain,
        LongDistanceTrain,
        Metro,
        RailShuttle,
        RapidTransit,
        Shuttle,
        Taxi,
        Train,
        Tramway,
    };
    Q_ENUM(Mode)
    Mode mode() const;
    void setMode(Mode mode);
    QString modeString() const;
    void setModeString(const QString &modeString);

    /** Checks if to instances refer to the same line (which does not necessarily mean they are exactly equal). */
    static bool isSame(const Line &lhs, const Line &rhs);

    /** Merge two Line instances.
     *  This assumes isSame(lhs, rhs) and tries to preserve the most detailed information.
     */
    static Line merge(const Line &lhs, const Line &rhs);
};

class RoutePrivate;

/** A route of a public transport line. */
class Route
{
    KPUBLICTRANSPORT_GADGET(Route)
    /** Line this route belongs to. */
    Q_PROPERTY(KPublicTransport::Line line READ line WRITE setLine)
    /** Direction of the route. */
    Q_PROPERTY(QString direction READ direction WRITE setDirection)

public:
    Line line() const;
    void setLine(const Line &line);
    QString direction() const;
    void setDirection(const QString &direction);

    /** Checks if to instances refer to the same route (which does not necessarily mean they are exactly equal). */
    static bool isSame(const Route &lhs, const Route &rhs);

    /** Merge two Route instances.
     *  This assumes isSame(lhs, rhs) and tries to preserve the most detailed information.
     */
    static Route merge(const Route &lhs, const Route &rhs);
};

}

Q_DECLARE_METATYPE(KPublicTransport::Line)
Q_DECLARE_METATYPE(KPublicTransport::Route)

#endif // KPUBLICTRANSPORT_LINE_H
