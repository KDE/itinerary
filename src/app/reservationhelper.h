/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef RESERVATIONHELPER_H
#define RESERVATIONHELPER_H

#include <qobjectdefs.h>

#include <utility>

class ReservationManager;

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

    /** Checks whether the given reservation is cancelled. */
    [[nodiscard]] static bool isCancelled(const QVariant &res);

    /** Display label for a given reservation. */
    Q_INVOKABLE [[nodiscard]] static QString label(const QVariant &res);

    /** Default icon name for the given element. */
    Q_INVOKABLE [[nodiscard]] static QString defaultIconName(const QVariant &res);

    /** Batch defining the location preceeding to @p resId.
     *  That's usually ReservationManager::previousBatch, apart from nested
     *  non-location change elements.
     */
    [[nodiscard]] static QString previousBatch(ReservationManager *resMgr, const QString &resId);
};

#endif // RESERVATIONHELPER_H
