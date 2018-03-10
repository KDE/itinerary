/*
   Copyright (c) 2017-2018 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#include "field.h"
#include "pass.h"
#include "pass_p.h"

#include <QGuiApplication>
#include <QJsonObject>

using namespace KPkPass;

namespace KPkPass {
class FieldPrivate {
public:
    const Pass *pass = nullptr;
    QJsonObject obj;
};
}

Field::Field()
    : d(new FieldPrivate)
{
}

Field::Field(const Field&) = default;
Field::Field(Field&&) = default;
Field::~Field() = default;
Field& Field::operator=(const Field&) = default;

Field::Field(const QJsonObject &obj, const Pass *pass)
    : d(new FieldPrivate)
{
    d->pass = pass;
    d->obj = obj;
}

QString Field::key() const
{
    return d->obj.value(QLatin1String("key")).toString();
}

QString Field::label() const
{
    if (d->pass) {
        return d->pass->d->message(d->obj.value(QLatin1String("label")).toString());
    }
    return {};
}

QVariant Field::value() const
{
    if (!d->pass) {
        return {};
    }
    auto v = d->pass->d->message(d->obj.value(QLatin1String("attributedValue")).toString());
    if (v.isEmpty()) {
        v = d->pass->d->message(d->obj.value(QLatin1String("value")).toString());
    }
    // TODO number and date/time detection
    return v;
}

QString Field::valueDisplayString() const
{
    // TODO respect number and date/time formatting options
    return value().toString();
}

QString Field::changeMessage() const
{
    if (!d->pass) {
        return {};
    }
    auto msg = d->pass->d->message(d->obj.value(QLatin1String("changeMessage")).toString());
    msg = msg.replace(QLatin1String("%@"), valueDisplayString());
    return msg;
}

Qt::Alignment Field::textAlignment() const
{
    const auto alignStr = d->obj.value(QLatin1String("textAlignment")).toString();
    if (alignStr == QLatin1String("PKTextAlignmentLeft")) {
        return Qt::AlignLeft;
    } else if (alignStr == QLatin1String("PKTextAlignmentCenter")) {
        return Qt::AlignHCenter;
    } else if (alignStr == QLatin1String("PKTextAlignmentRight")) {
        return Qt::AlignRight;
    }
    return QGuiApplication::layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight;
}
