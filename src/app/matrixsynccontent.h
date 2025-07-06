/*
    SPDX-FileCopyrightText: ⓒ 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MATRIXSYNCCONTENT_H
#define MATRIXSYNCCONTENT_H

#include <config-itinerary.h>

#include "transfer.h"

#include <QString>

class DocumentManager;
class LiveDataManager;
class MatrixSyncStateEvent;
class PkPassManager;
class ReservationManager;
class TransferManager;

using namespace Qt::Literals;

/** Matrix-based trip synchronization de/serialization methods. */
namespace MatrixSyncContent
{
#if HAVE_MATRIX
    /** Create a state event for a given reservation batch. */
    [[nodiscard]] MatrixSyncStateEvent stateEventForBatch(const QString &batchId, const ReservationManager *resMgr);
    [[nodiscard]] MatrixSyncStateEvent stateEventForDeletedBatch(const QString &batchId);

    /** Read reservation batch state event. */
    [[nodiscard]] QString readBatch(const MatrixSyncStateEvent &event, ReservationManager *resMgr);

    /** Create a state event for live data for a given batch. */
    [[nodiscard]] MatrixSyncStateEvent stateEventForLiveData(const QString &batchId);

    /** Read live data for a batch from a state event. */
    void readLiveData(const MatrixSyncStateEvent &event, LiveDataManager *ldm);

    /** Create a state event for a transfer. */
    [[nodiscard]] MatrixSyncStateEvent stateEventForTransfer(const QString &batchId, Transfer::Alignment alignment, const TransferManager *transferMgr);

    /** Read transfer data from a state event. */
    void readTransfer(const MatrixSyncStateEvent &event, TransferManager *transferMgr);

    /** Create a state event for an uploaded document. */
    [[nodiscard]] MatrixSyncStateEvent stateEventForDocument(const QString &docId, const DocumentManager *docMgr);

    /** Read document from state event. */
    void readDocument(const MatrixSyncStateEvent &event, DocumentManager *docMgr);

    /** Create a state event for an uploaded pkpass. */
    [[nodiscard]] MatrixSyncStateEvent stateEventForPkPass(const QString &passId);

    /** Read pkpass from state event. */
    void readPkPass(const MatrixSyncStateEvent &event, PkPassManager *pkPassMgr);
#endif
}

#endif
