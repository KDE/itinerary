/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef RESERVATIONHELPER_H
#define RESERVATIONHELPER_H

#include <utility>

class QDateTime;
class QString;
class QVariant;

/** Helper methods for dealing with KItinerary reservation elements. */
namespace ReservationHelper
{
    /** Type registration setup.
     *  @todo This should happen automatically in KItinerary!
     */
    void setup();

    std::pair<QString, QString> lineNameAndNumber(const QVariant &res);

    bool equals(const QVariant &lhs, const QVariant &rhs);

    /** Returns the arrival/departure time if available.
     *  This returns an invalid time for unbound train tickets or bare flight boarding passes.
     */
    QDateTime departureTime(const QVariant &res);
    QDateTime arrivalTime(const QVariant &res);
}

#endif // RESERVATIONHELPER_H
