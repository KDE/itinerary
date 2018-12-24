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

#include <QJsonObject>

class QMetaObject;

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

    void fromJson(const QMetaObject *mo, const QJsonObject &obj, void *elem);

    /** Deserialize via QMetaObject. */
    template <typename T> inline T fromJson(const QJsonObject &obj)
    {
        T elem;
        fromJson(&T::staticMetaObject, obj, &elem);
        return elem;
    }
}

}

#endif // KPUBLICTRANSPORT_JSON_H
