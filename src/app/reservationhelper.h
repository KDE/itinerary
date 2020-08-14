/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
