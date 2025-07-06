/*
    SPDX-FileCopyrightText: ⓒ 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matrixsynccontent.h"

#include "livedatamanager.h"
#include "logging.h"
#include "reservationmanager.h"
#include "transfermanager.h"

#include <KItinerary/JsonLdDocument>

#if HAVE_MATRIX
#include <Quotient/events/stateevent.h>

#include <QJsonDocument>
#include <QJsonObject>

constexpr inline auto ContentKey = "content"_L1;

std::unique_ptr<Quotient::StateEvent> MatrixSyncContent::stateEventForBatch(const QString &batchId, const ReservationManager *resMgr)
{
    const auto reservationIds = resMgr->reservationsForBatch(batchId);
    QJsonObject content;
    for (const auto &resId : reservationIds) {
        content.insert(resId, KItinerary::JsonLdDocument::toJson(resMgr->reservation(resId)));
    }

    auto state = std::make_unique<Quotient::StateEvent>(MatrixSyncContent::ReservationEventType, batchId, QJsonObject({
        {ContentKey, QString::fromUtf8(QJsonDocument(content).toJson(QJsonDocument::Compact))}
    }));

    return state;
}

std::unique_ptr<Quotient::StateEvent> MatrixSyncContent::stateEventForDeletedBatch(const QString &batchId)
{
    return std::make_unique<Quotient::StateEvent>(MatrixSyncContent::ReservationEventType, batchId, QJsonObject{});
}

QString MatrixSyncContent::readBatch(const Quotient::StateEvent *event, ReservationManager *resMgr)
{
    auto batchId = event->stateKey();
    const auto content = QJsonDocument::fromJson(event->contentJson().value(ContentKey).toString().toUtf8()).object();
    if (content.isEmpty()) {
        // deleted
        resMgr->removeBatch(batchId);
        return {};
    }

    std::vector<ReservationManager::ReservationChange> changes;
    changes.reserve(content.size());
    changes.emplace_back(event->stateKey(), KItinerary::JsonLdDocument::fromJsonSingular(content.value(batchId).toObject()));
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

std::unique_ptr<Quotient::StateEvent> MatrixSyncContent::stateEventForLiveData(const QString &batchId)
{
    const auto ld = LiveData::load(batchId);
    auto state = std::make_unique<Quotient::StateEvent>(MatrixSyncContent::LiveDataEventType, batchId, QJsonObject({
        {ContentKey, QString::fromUtf8(QJsonDocument(LiveData::toJson(ld)).toJson(QJsonDocument::Compact))}
    }));
    return state;
}

void MatrixSyncContent::readLiveData(const Quotient::StateEvent *event, LiveDataManager *ldm)
{
    const auto batchId = event->stateKey();
    const auto content = QJsonDocument::fromJson(event->contentJson().value(ContentKey).toString().toUtf8()).object();

    auto ld = LiveData::fromJson(content);
    ld.store(batchId);
    ldm->importData(batchId, std::move(ld));
}

std::unique_ptr<Quotient::StateEvent>MatrixSyncContent::stateEventForTransfer(const QString &batchId, Transfer::Alignment alignment, const TransferManager *transferMgr)
{
    const auto transfer = transferMgr->transfer(batchId, alignment);
    auto state = std::make_unique<Quotient::StateEvent>(MatrixSyncContent::TransferEventType, Transfer::identifier(batchId, alignment), QJsonObject({
        {ContentKey, QString::fromUtf8(QJsonDocument(Transfer::toJson(transfer)).toJson(QJsonDocument::Compact))}
    }));
    return state;
}

void MatrixSyncContent::readTransfer(const Quotient::StateEvent *event, TransferManager *transferMgr)
{
    const auto [batchId, alignment] = Transfer::parseIdentifier(event->stateKey());
    if (batchId.isEmpty()) {
        return;
    }
    const auto content = QJsonDocument::fromJson(event->contentJson().value(ContentKey).toString().toUtf8()).object();
    const auto transfer = Transfer::fromJson(content);

    if (transfer.state() == Transfer::UndefinedState) {
        transferMgr->removeTransfer(batchId, alignment);
    } else {
        transferMgr->importTransfer(transfer);
    }
}

#endif
