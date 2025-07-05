/*
    SPDX-FileCopyrightText: ⓒ 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matrixsynccontent.h"

#include "logging.h"

#if HAVE_MATRIX
#include "reservationmanager.h"

#include <KItinerary/JsonLdDocument>

#include <Quotient/events/stateevent.h>

#include <QJsonDocument>
#include <QJsonObject>

constexpr inline auto BatchContentKey = "content"_L1;

std::unique_ptr<Quotient::StateEvent> MatrixSyncContent::stateEventForBatch(const QString &batchId, const ReservationManager *resMgr)
{
    const auto reservationIds = resMgr->reservationsForBatch(batchId);
    QJsonObject content;
    for (const auto &resId : reservationIds) {
        content.insert(resId, KItinerary::JsonLdDocument::toJson(resMgr->reservation(resId)));
    }

    auto state = std::make_unique<Quotient::StateEvent>(MatrixSyncContent::ReservationEventType, batchId, QJsonObject({
        {BatchContentKey, QString::fromUtf8(QJsonDocument(content).toJson(QJsonDocument::Compact))}
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
    const auto content = QJsonDocument::fromJson(event->contentJson()[BatchContentKey].toString().toUtf8()).object();
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

#endif
