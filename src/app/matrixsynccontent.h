/*
    SPDX-FileCopyrightText: ⓒ 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MATRIXSYNCCONTENT_H
#define MATRIXSYNCCONTENT_H

#include <config-itinerary.h>

#include "transfer.h"

#if HAVE_MATRIX
#include <Quotient/events/filesourceinfo.h>
#endif

#include <QString>

class DocumentManager;
class LiveDataManager;
class ReservationManager;
class TransferManager;

namespace Quotient {
class StateEvent;
}

using namespace Qt::Literals;

/** Matrix-based trip synchronization de/serialization methods. */
namespace MatrixSyncContent
{
    constexpr inline auto ReservationEventType = "org.kde.itinerary.reservation"_L1;
    constexpr inline auto LiveDataEventType = "org.kde.itinerary.livedata"_L1;
    constexpr inline auto TransferEventType = "org.kde.itinerary.transfer"_L1;
    constexpr inline auto DocumentEventType = "org.kde.itinerary.document"_L1;
    constexpr inline auto PkPassEventType = "org.kde.itinerary.pkpass"_L1;

#if HAVE_MATRIX
    /** Create a state event for a given reservation batch. */
    [[nodiscard]] std::unique_ptr<Quotient::StateEvent> stateEventForBatch(const QString &batchId, const ReservationManager *resMgr);
    [[nodiscard]] std::unique_ptr<Quotient::StateEvent> stateEventForDeletedBatch(const QString &batchId);

    /** Read reservation batch state event. */
    [[nodiscard]] QString readBatch(const Quotient::StateEvent *event, ReservationManager *resMgr);

    /** Create a state event for live data for a given batch. */
    [[nodiscard]] std::unique_ptr<Quotient::StateEvent> stateEventForLiveData(const QString &batchId);

    /** Read live data for a batch from a state event. */
    void readLiveData(const Quotient::StateEvent *event, LiveDataManager *ldm);

    /** Create a state event for a transfer. */
    [[nodiscard]] std::unique_ptr<Quotient::StateEvent> stateEventForTransfer(const QString &batchId, Transfer::Alignment alignment, const TransferManager *transferMgr);

    /** Read transfer data from a state event. */
    void readTransfer(const Quotient::StateEvent *event, TransferManager *transferMgr);

    /** Create a state event for an uploaded document. */
    [[nodiscard]] std::unique_ptr<Quotient::StateEvent> stateEventForDocument(const QString &docId, const Quotient::FileSourceInfo &info, const DocumentManager *docMgr);

    /** Create a state event for an uploaded pkpass. */
    [[nodiscard]] std::unique_ptr<Quotient::StateEvent> stateEventForPkPass(const QString &passId, const Quotient::FileSourceInfo &info);
#endif
}

#endif
