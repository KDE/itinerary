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
    [[nodiscard]] std::pair<QString, QString> lineNameAndNumber(const QVariant &res);

    [[nodiscard]] bool equals(const QVariant &lhs, const QVariant &rhs);

    /** Returns the UIC company code for @p res, if any. */
    [[nodiscard]] QString uicCompanyCode(const QVariant &res);
    /** Returns the VDV org id for @p res, if any. */
    [[nodiscard]] QString vdvOrganizationId(const QVariant &res);

    /** Returns whether @p res is an unbound reservation. */
    [[nodiscard]] bool isUnbound(const QVariant &res);

    /** Checks whether the given reservation is canclled. */
    [[nodiscard]] bool isCancelled(const QVariant &res);
}

#endif // RESERVATIONHELPER_H
