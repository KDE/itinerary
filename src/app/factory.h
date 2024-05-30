/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef FACTORY_H
#define FACTORY_H

#include "qobjectdefs.h"

class QVariant;

/** QML factory functions for gadget types. */
class Factory
{
    Q_GADGET
public:
    Q_INVOKABLE [[nodiscard]] static QVariant makeFlightReservation();
    Q_INVOKABLE [[nodiscard]] static QVariant makeBoatReservation();
    Q_INVOKABLE [[nodiscard]] static QVariant makeEventReservation();
    Q_INVOKABLE [[nodiscard]] static QVariant makeFoodEstablishmentReservation();
    Q_INVOKABLE [[nodiscard]] static QVariant makeLodgingReservation();
    Q_INVOKABLE [[nodiscard]] static QVariant makePerson();
    Q_INVOKABLE [[nodiscard]] static QVariant makePlace();
    Q_INVOKABLE [[nodiscard]] static QVariant makeProgramMembership();
    Q_INVOKABLE [[nodiscard]] static QVariant makeTicket();
};

#endif // FACTORY_H
