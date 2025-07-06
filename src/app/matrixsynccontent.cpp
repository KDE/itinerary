/*
    SPDX-FileCopyrightText: ⓒ 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matrixsynccontent.h"

#include "livedatamanager.h"
#include "logging.h"
#include "matrixsyncstateevent.h"
#include "reservationmanager.h"
#include "transfermanager.h"

#include <KItinerary/JsonLdDocument>

#if HAVE_MATRIX
#include <Quotient/events/stateevent.h>

#include <QJsonDocument>
#include <QJsonObject>

MatrixSyncStateEvent MatrixSyncContent::stateEventForBatch(const QString &batchId, const ReservationManager *resMgr)
{
    const auto reservationIds = resMgr->reservationsForBatch(batchId);
    QJsonObject content;
    for (const auto &resId : reservationIds) {
        content.insert(resId, KItinerary::JsonLdDocument::toJson(resMgr->reservation(resId)));
    }

    MatrixSyncStateEvent state(MatrixSync::ReservationEventType, batchId);
    state.setContent(QJsonDocument(content).toJson(QJsonDocument::Compact));
    return state;
}

MatrixSyncStateEvent MatrixSyncContent::stateEventForDeletedBatch(const QString &batchId)
{
    return MatrixSyncStateEvent(MatrixSync::ReservationEventType, batchId);
}

QString MatrixSyncContent::readBatch(const MatrixSyncStateEvent &event, ReservationManager *resMgr)
{
    auto batchId = event.stateKey();
    const auto content = QJsonDocument::fromJson(event.content()).object();
    if (content.isEmpty()) {
        // deleted
        resMgr->removeBatch(batchId);
        return {};
    }

    std::vector<ReservationManager::ReservationChange> changes;
    changes.reserve(content.size());
    changes.emplace_back(batchId, KItinerary::JsonLdDocument::fromJsonSingular(content.value(batchId).toObject()));
    for (auto it = content.begin(); it != content.end(); ++it) {
        if (it.key() == batchId) {
            continue;
        }
        changes.emplace_back(it.key(), KItinerary::JsonLdDocument::fromJsonSingular(it.value().toObject()));
    }

    if (resMgr->hasBatch(batchId)) {
        qCDebug(Log) << "updating reservation" << batchId;
        resMgr->updateBatch(changes);
        qCDebug(Log) << "updated reservation" << batchId;
    } else {
        qCDebug(Log) << "creating reservation" << batchId;
        // TODO atomic API for this?
        for (const auto &change : changes) {
            resMgr->addReservation(change.res, change.id);
        }
        qCDebug(Log) << "created reservation" << batchId;
    }

    return batchId;
}

MatrixSyncStateEvent MatrixSyncContent::stateEventForLiveData(const QString &batchId)
{
    const auto ld = LiveData::load(batchId);
    MatrixSyncStateEvent state(MatrixSync::LiveDataEventType, batchId);
    state.setContent(QJsonDocument(LiveData::toJson(ld)).toJson(QJsonDocument::Compact));
    return state;
}

void MatrixSyncContent::readLiveData(const MatrixSyncStateEvent &event, LiveDataManager *ldm)
{
    const auto batchId = event.stateKey();
    const auto content = QJsonDocument::fromJson(event.content()).object();

    auto ld = LiveData::fromJson(content);
    ld.store(batchId);
    ldm->importData(batchId, std::move(ld));
}

MatrixSyncStateEvent MatrixSyncContent::stateEventForTransfer(const QString &batchId, Transfer::Alignment alignment, const TransferManager *transferMgr)
{
    const auto transfer = transferMgr->transfer(batchId, alignment);
    MatrixSyncStateEvent state(MatrixSync::TransferEventType, Transfer::identifier(batchId, alignment));
    state.setContent(QJsonDocument(Transfer::toJson(transfer)).toJson(QJsonDocument::Compact));
    return state;
}

void MatrixSyncContent::readTransfer(const MatrixSyncStateEvent &event, TransferManager *transferMgr)
{
    const auto [batchId, alignment] = Transfer::parseIdentifier(event.stateKey());
    if (batchId.isEmpty()) {
        return;
    }
    const auto content = QJsonDocument::fromJson(event.content()).object();
    const auto transfer = Transfer::fromJson(content);

    if (transfer.state() == Transfer::UndefinedState) {
        transferMgr->removeTransfer(batchId, alignment);
    } else {
        transferMgr->importTransfer(transfer);
    }
}

#endif
