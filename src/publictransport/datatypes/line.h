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

public:
    QString name() const;
    void setName(const QString &name);
    QColor color() const;
    void setColor(const QColor &color);
    QColor textColor() const;
    void setTextColor(const QColor &textColor);
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
};

}

Q_DECLARE_METATYPE(KPublicTransport::Line)
Q_DECLARE_METATYPE(KPublicTransport::Route)

#endif // KPUBLICTRANSPORT_LINE_H
