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
#include "json.h"

#include <QColor>
#include <QDebug>

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

static bool isSameLocationName(const QString &lhs, const QString &rhs)
{
    if (lhs.size() == rhs.size()) {
        return lhs.compare(rhs, Qt::CaseInsensitive) == 0;
    }
    if (lhs.size() < rhs.size()) {
        return rhs.startsWith(lhs, Qt::CaseInsensitive);
    }
    return lhs.startsWith(rhs, Qt::CaseInsensitive);
}

static bool isSameLineName(const QString &lhs, const QString &rhs)
{
    if (lhs.size() == rhs.size()) {
        return lhs.compare(rhs, Qt::CaseInsensitive) == 0;
    }
    if (lhs.size() < rhs.size()) {
        return rhs.endsWith(QLatin1Char(' ') + lhs, Qt::CaseInsensitive);
    }
    return lhs.endsWith(QLatin1Char(' ') + rhs, Qt::CaseInsensitive);
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

bool Line::isSame(const Line &lhs, const Line &rhs)
{
    return isSameLineName(lhs.name(), rhs.name()) && lhs.mode() == rhs.mode();
}

Line Line::merge(const Line &lhs, const Line &rhs)
{
    Line l(lhs);
    if (!l.color().isValid() && rhs.color().isValid()) {
        l.setColor(rhs.color());
    }
    if (!l.textColor().isValid() && rhs.textColor().isValid()) {
        l.setTextColor(rhs.textColor());
    }
    return l;
}

QJsonObject Line::toJson(const Line &l)
{
    auto obj = Json::toJson(l);
    return obj;
}

Line Line::fromJson(const QJsonObject &obj)
{
    auto l = Json::fromJson<Line>(obj);
    return l;
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

bool Route::isSame(const Route &lhs, const Route &rhs)
{
    return isSameLocationName(lhs.direction(), rhs.direction()) &&
        Line::isSame(lhs.line(), rhs.line());
}

Route Route::merge(const Route &lhs, const Route &rhs)
{
    Route r(lhs);
    r.setLine(Line::merge(lhs.line(), rhs.line()));
    return r;
}

QJsonObject Route::toJson(const Route &r)
{
    auto obj = Json::toJson(r);
    obj.insert(QLatin1String("line"), Line::toJson(r.line()));
    return obj;
}

Route Route::fromJson(const QJsonObject &obj)
{
    auto r = Json::fromJson<Route>(obj);
    r.setLine(Line::fromJson(obj.value(QLatin1String("line")).toObject()));
    return r;
}


#include "moc_line.cpp"
