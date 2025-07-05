/*
    SPDX-FileCopyrightText: ⓒ 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MATRIXSYNCCONTENT_H
#define MATRIXSYNCCONTENT_H

#include <config-itinerary.h>

#include <QString>

class ReservationManager;

namespace Quotient {
    class StateEvent;
}

using namespace Qt::Literals;

/** Matrix-based trip synchronization de/serialization methods. */
namespace MatrixSyncContent
{
    constexpr inline auto ReservationEventType = "org.kde.itinerary.reservation"_L1;

    /** Create a state event for a given reservation batch. */
    [[nodiscard]] std::unique_ptr<Quotient::StateEvent> stateEventForBatch(const QString &batchId, const ReservationManager *resMgr);
    [[nodiscard]] std::unique_ptr<Quotient::StateEvent> stateEventForDeletedBatch(const QString &batchId);

    /** Read reservation batch state event. */
    [[nodiscard]] QString readBatch(const Quotient::StateEvent *event, ReservationManager *resMgr);
}

#endif
