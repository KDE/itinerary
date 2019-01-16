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

#ifndef KPUBLICTRANSPORT_JSON_H
#define KPUBLICTRANSPORT_JSON_H

#include <QJsonArray>
#include <QJsonObject>

#include <vector>

struct QMetaObject;

namespace KPublicTransport {

/** De/serialization helper methods. */
namespace Json
{
    QJsonObject toJson(const QMetaObject *mo, const void *elem);

    /** Serialize from QMetaObject. */
    template <typename T> inline QJsonObject toJson(const T &elem)
    {
        return toJson(&T::staticMetaObject, &elem);
    }

    /** Serialize an array of elements. */
    template <typename T> inline QJsonArray toJson(const std::vector<T> &elems)
    {
        QJsonArray a;
        //a.reserve(elems.size());
        std::transform(elems.begin(), elems.end(), std::back_inserter(a), QOverload<const T&>::of(&T::toJson));
        return a;
    }

    void fromJson(const QMetaObject *mo, const QJsonObject &obj, void *elem);

    /** Deserialize via QMetaObject. */
    template <typename T> inline T fromJson(const QJsonObject &obj)
    {
        T elem;
        fromJson(&T::staticMetaObject, obj, &elem);
        return elem;
    }

    /** Deserialize an array of elements. */
    template <typename T> inline std::vector<T> fromJson(const QJsonArray &a)
    {
        std::vector<T> res;
        res.reserve(a.size());
        std::transform(a.begin(), a.end(), std::back_inserter(res), [](const auto &v) { return T::fromJson(v.toObject()); });
        return res;
    }
}

}

#endif // KPUBLICTRANSPORT_JSON_H
