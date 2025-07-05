/*
    SPDX-FileCopyrightText: ⓒ 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MATRIXSYNCCONTENT_H
#define MATRIXSYNCCONTENT_H

#include <config-itinerary.h>

#include <QString>

class MatrixSyncStateEvent;
class ReservationManager;

using namespace Qt::Literals;

/** Matrix-based trip synchronization de/serialization methods. */
namespace MatrixSyncContent
{
    /** Create a state event for a given reservation batch. */
    [[nodiscard]] MatrixSyncStateEvent stateEventForBatch(const QString &batchId, const ReservationManager *resMgr);
    [[nodiscard]] MatrixSyncStateEvent stateEventForDeletedBatch(const QString &batchId);

    /** Read reservation batch state event. */
    [[nodiscard]] QString readBatch(const MatrixSyncStateEvent &event, ReservationManager *resMgr);
}

#endif
