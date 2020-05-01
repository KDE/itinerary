/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

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

#ifndef RESERVATIONHELPER_H
#define RESERVATIONHELPER_H

#include <utility>

class QString;
class QVariant;

/** Helper methods for dealing with KItinerary reservation elements. */
namespace ReservationHelper
{
    std::pair<QString, QString> lineNameAndNumber(const QVariant &res);

    bool equals(const QVariant &lhs, const QVariant &rhs);
}

#endif // RESERVATIONHELPER_H
