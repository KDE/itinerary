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

#include "line.h"
#include "datatypes_p.h"

#include <QColor>

using namespace KPublicTransport;

namespace KPublicTransport {
class LinePrivate : public QSharedData {
public:
    Line::Mode mode;
    QString modeString;
    QString name;
    QColor color;
    QColor textColor;
};

class RoutePrivate : public QSharedData {
public:
    Line line;
    QString direction;
};

}

KPUBLICTRANSPORT_MAKE_GADGET(Line)

QString Line::name() const
{
    return d->name;
}

void Line::setName(const QString &name)
{
    d.detach();
    d->name = name;
}

QColor Line::color() const
{
    return d->color;
}

void Line::setColor(const QColor &color)
{
    d.detach();
    d->color = color;
}

QColor Line::textColor() const
{
    return d->textColor;
}

void Line::setTextColor(const QColor &textColor)
{
    d.detach();
    d->textColor = textColor;
}

Line::Mode Line::mode() const
{
    return d->mode;
}

void Line::setMode(Line::Mode mode)
{
    d.detach();
    d->mode = mode;
}

QString Line::modeString() const
{
    return d->modeString;
}

void Line::setModeString(const QString &modeString)
{
    d.detach();
    d->modeString = modeString;
}


KPUBLICTRANSPORT_MAKE_GADGET(Route)

Line Route::line() const
{
    return d->line;
}

void Route::setLine(const Line &line)
{
    d.detach();
    d->line = line;
}

QString Route::direction() const
{
    return d->direction;
}

void Route::setDirection(const QString &direction)
{
    d.detach();
    d->direction = direction;
}

#include "moc_line.cpp"
