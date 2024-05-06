/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef RESERVATIONHELPER_H
#define RESERVATIONHELPER_H

#include <qobjectdefs.h>

#include <utility>

class QString;
class QVariant;

/** Helper methods for dealing with KItinerary reservation elements. */
class ReservationHelper
{
    Q_GADGET
public:
    [[nodiscard]] static std::pair<QString, QString> lineNameAndNumber(const QVariant &res);

    [[nodiscard]] static bool equals(const QVariant &lhs, const QVariant &rhs);

    /** Returns the UIC company code for @p res, if any. */
    [[nodiscard]] static QString uicCompanyCode(const QVariant &res);
    /** Returns the VDV org id for @p res, if any. */
    [[nodiscard]] static QString vdvOrganizationId(const QVariant &res);

    /** Returns whether @p res is an unbound reservation. */
    [[nodiscard]] static bool isUnbound(const QVariant &res);

    /** Checks whether the given reservation is canclled. */
    [[nodiscard]] static bool isCancelled(const QVariant &res);

    /** Default icon name for the given element. */
    Q_INVOKABLE [[nodiscard]] static QString defaultIconName(const QVariant &res);
};

#endif // RESERVATIONHELPER_H
