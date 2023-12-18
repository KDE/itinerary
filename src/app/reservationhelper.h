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

    /** Returns the UIC company code for @p res, if any. */
    QString uicCompanyCode(const QVariant &res);
    /** Returns the VDV org id for @p res, if any. */
    QString vdvOrganizationId(const QVariant &res);

    /** Returns whether @p res is an unbound reservation. */
    bool isUnbound(const QVariant &res);

    /** Checks whether the given reservation is canclled. */
    bool isCancelled(const QVariant &res);
}

#endif // RESERVATIONHELPER_H
