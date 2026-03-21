/*
    SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SCAMWARNINGMANAGER_H
#define SCAMWARNINGMANAGER_H

#include <QVariant>

/** Warnings for scam airports, and persistence for ignoring those per trip or permanently. */
class ScamWarningManager
{
    Q_GADGET
public:
    /** Show scam warning for the given place. */
    Q_INVOKABLE [[nodiscard]] static bool warnForPlace(const QVariant &place, const QString &tripId);
    /** Ignore scam warning about @p place for trip @p tripId. */
    Q_INVOKABLE static void ignorePlaceForTrip(const QVariant &place, const QString &tripId);
    /** Ignore scam warning about @p place indefinitely. */
    Q_INVOKABLE static void ignorePlacePermanently(const QVariant &place);

    /** Connect to trip group manager for cleaning up ignore states. */
    static void tripRemoved(const QString &tripId);
};

#endif
