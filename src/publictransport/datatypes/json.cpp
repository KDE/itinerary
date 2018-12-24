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

#include "json.h"

#include <QMetaObject>
#include <QMetaProperty>
#include <QVariant>

using namespace KPublicTransport;

static QJsonValue variantToJson(const QVariant &v)
{
    switch (v.userType()) {
        case QMetaType::QString:
            return v.toString();
        case QMetaType::Double:
        case QMetaType::Float:
            return v.toDouble();
        case QMetaType::Int:
            return v.toInt();
    }

    return {};
}

QJsonObject Json::toJson(const QMetaObject *mo, const void *elem)
{
    QJsonObject obj;

    for (int i = 0; i < mo->propertyCount(); ++i) {
        const auto prop = mo->property(i);
        if (!prop.isStored()) {
            continue;
        }

        const auto v = variantToJson(prop.readOnGadget(elem));
        if (!v.isNull()) {
            obj.insert(QString::fromUtf8(prop.name()), v);
        }
    }

    return obj;
}

static QVariant variantFromJson(const QJsonValue &v, int mt)
{
    switch (mt) {
        case QMetaType::QString:
            return v.toString();
        case QMetaType::Double:
        case QMetaType::Float:
            return v.toDouble();
        case QMetaType::Int:
            return v.toInt();
    }

    return {};
}

void Json::fromJson(const QMetaObject *mo, const QJsonObject &obj, void *elem)
{
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        const auto idx = mo->indexOfProperty(it.key().toUtf8());
        if (idx < 0) {
            continue;
        }

        const auto prop = mo->property(idx);
        if (!prop.isStored()) {
            continue;
        }

        const auto v = variantFromJson(it.value(), prop.userType());
        prop.writeOnGadget(elem, v);
    }
}
