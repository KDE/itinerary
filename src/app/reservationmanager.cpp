/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reservationmanager.h"

#include "jsonio.h"
#include "logging.h"
#include "reservationhelper.h"

#include <KItinerary/Event>
#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/MergeUtil>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>
#include <KItinerary/Visit>

#include <KLocalizedString>

#include <QDate>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QScopeGuard>
#include <QStandardPaths>
#include <QUrl>
#include <QUuid>

using namespace KItinerary;
using namespace Qt::Literals;

ReservationManager::ReservationManager(QObject *parent)
    : QObject(parent)
    , m_validator(ReservationManager::validator())
{
    m_validator.setAcceptOnlyCompleteElements(true);

    loadBatches();
}

ReservationManager::~ReservationManager() = default;

KItinerary::ExtractorValidator ReservationManager::validator()
{
    KItinerary::ExtractorValidator v;
    v.setAcceptedTypes<BoatReservation,
                       BusReservation,
                       EventReservation,
                       FlightReservation,
                       FoodEstablishmentReservation,
                       LodgingReservation,
                       RentalCarReservation,
                       TrainReservation,
                       TouristAttractionVisit>();
    return v;
}

bool ReservationManager::isEmpty() const
{
    return m_batchToResMap.empty();
}

bool ReservationManager::hasBatch(const QString &batchId) const
{
    return m_batchToResMap.contains(batchId);
}

QVariant ReservationManager::reservation(const QString &id) const
{
    if (id.isEmpty()) {
        return {};
    }

    const auto it = m_reservations.constFind(id);
    if (it != m_reservations.constEnd()) {
        return it.value();
    }

    const QString resPath = reservationsBasePath() + id + QLatin1StringView(".jsonld");
    QFile f(resPath);
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to open JSON-LD reservation data file:" << resPath << f.errorString();
        return {};
    }

    const auto val = JsonIO::read(f.readAll());
    if (!(val.isArray() && val.toArray().size() == 1) && !val.isObject()) {
        qCWarning(Log) << "Invalid JSON-LD reservation data file:" << resPath;
        return {};
    }

    const auto resData = JsonLdDocument::fromJson(val.isArray() ? val.toArray() : QJsonArray({val.toObject()}));
    if (resData.size() != 1) {
        qCWarning(Log) << "Unable to parse JSON-LD reservation data file:" << resPath;
        return {};
    }

    // re-run post-processing to benefit from newer augmentations
    ExtractorPostprocessor postproc;
    postproc.process(resData);
    if (postproc.result().size() != 1) {
        qCWarning(Log) << "Post-processing discarded the reservation:" << resPath;
        return {};
    }

    const auto res = postproc.result().at(0);
    if (!m_validator.isValidElement(res)) {
        qCWarning(Log) << "Validation discarded the reservation:" << resPath;
        return {};
    }
    m_reservations.insert(id, res);
    return res;
}

QString ReservationManager::reservationsBasePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1StringView("/reservations/");
}

QString ReservationManager::batchesBasePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1StringView("/batches/");
}

QList<QString> ReservationManager::addReservationsWithPostProcessing(const QList<QVariant> &resData)
{
    ExtractorPostprocessor postproc;
    postproc.setContextDate(QDateTime(QDate::currentDate(), QTime(0, 0)));
    postproc.process(resData);

    auto data = postproc.result();
    QList<QString> ids;
    ids.reserve(data.size());
    for (auto &res : data) {
        if (JsonLd::isA<Event>(res)) { // promote Event to EventReservation
            EventReservation ev;
            ev.setReservationFor(res);
            ev.setPotentialAction(res.value<Event>().potentialAction());
            res = ev;
        }

        ids.push_back(addReservation(res));
    }

    return ids;
}

QString ReservationManager::addReservation(const QVariant &res, const QString &resIdHint)
{
    // deal with partial updates
    if (!m_validator.isValidElement(res)) {
        const auto cleanup = qScopeGuard([this] {
            m_validator.setAcceptOnlyCompleteElements(true);
        });
        m_validator.setAcceptOnlyCompleteElements(false);
        if (m_validator.isValidElement(res)) {
            const auto ids = applyPartialUpdate(res);
            return ids.empty() ? QString() : ids.front();
        }
        qCWarning(Log) << "Discarding added element due to validation failure" << res;
        return {};
    }

    // look for matching reservations, or matching batches
    // we need to do that within a +/-24h range, so we find unbound elements too
    // TODO in case this updates the time for an unbound element we need to re-sort, otherwise the prev/next logic fails!
    const auto rangeBegin = SortUtil::startDateTime(res).addDays(-1);
    const auto rangeEnd = rangeBegin.addDays(2);

    const auto beginIt = std::lower_bound(m_batches.begin(), m_batches.end(), rangeBegin, [this](const auto &lhs, const auto &rhs) {
        return SortUtil::startDateTime(reservation(lhs)) < rhs;
    });
    for (auto it = beginIt; it != m_batches.end(); ++it) {
        const auto otherRes = reservation(*it);
        if (SortUtil::startDateTime(otherRes) > rangeEnd) {
            break; // no hit
        }
        if (MergeUtil::isSame(res, otherRes)) {
            // this is actually an update of otherRes!
            const auto newRes = MergeUtil::merge(otherRes, res);
            updateReservation(*it, newRes);
            return *it;
        }
        if (MergeUtil::isSameIncidence(res, otherRes)) {
            // this is a multi-traveler element, check if we have it as one of the batch elements already
            const auto &batch = m_batchToResMap.value(*it);
            for (const auto &batchedId : batch) {
                const auto batchedRes = reservation(batchedId);
                if (MergeUtil::isSame(res, batchedRes)) {
                    // this is actually an update of a batched reservation
                    const auto newRes = MergeUtil::merge(otherRes, res);
                    updateReservation(batchedId, newRes);
                    return batchedId;
                }
            }

            // truly new, and added to an existing batch
            const QString resId = makeReservationId(resIdHint);
            storeReservation(resId, res);
            Q_EMIT reservationAdded(resId);

            m_batchToResMap[*it].push_back(resId);
            m_resToBatchMap.insert(resId, *it);
            Q_EMIT batchChanged(*it);
            storeBatch(*it);
            return resId;
        }
    }

    // truly new, and starting a new batch
    const QString resId = makeReservationId(resIdHint);
    storeReservation(resId, res);
    Q_EMIT reservationAdded(resId);

    // search for the precise insertion place, beginIt is only the begin of our initial search range
    const auto insertIt = std::lower_bound(m_batches.begin(), m_batches.end(), SortUtil::startDateTime(res), [this](const auto &lhs, const auto &rhs) {
        return SortUtil::startDateTime(reservation(lhs)) < rhs;
    });
    m_batches.insert(insertIt, resId);
    m_batchToResMap.insert(resId, {resId});
    m_resToBatchMap.insert(resId, resId);
    Q_EMIT batchAdded(resId);
    storeBatch(resId);
    return resId;
}

QString ReservationManager::addReservationWithPostProcessing(const QVariant &resData)
{
    const auto l = addReservationsWithPostProcessing({resData});
    return !l.isEmpty() ? l.at(0) : QString();
}

void ReservationManager::updateReservation(const QString &resId, const QVariant &res)
{
    const auto oldRes = reservation(resId);

    storeReservation(resId, res);
    Q_EMIT reservationChanged(resId);

    updateBatch(resId, res, oldRes);
}

void ReservationManager::storeReservation(const QString &resId, const QVariant &res) const
{
    const QString basePath = reservationsBasePath();
    QDir::root().mkpath(basePath);
    const QString path = basePath + resId + QLatin1StringView(".jsonld");
    QFile f(path);
    if (!f.open(QFile::WriteOnly)) {
        qCWarning(Log) << "Unable to open file:" << f.errorString();
        return;
    }
    f.write(JsonIO::write(JsonLdDocument::toJson(res)));
    m_reservations.insert(resId, res);
}

void ReservationManager::removeReservation(const QString &id)
{
    const auto batchId = m_resToBatchMap.value(id);
    removeFromBatch(id, batchId);

    const QString basePath = reservationsBasePath();
    if (!QFile::remove(basePath + '/'_L1 + id + ".jsonld"_L1)) {
        qCWarning(Log) << "Failed to remove file:" << id;
    }
    Q_EMIT reservationRemoved(id);
    m_reservations.remove(id);
}

const std::vector<QString> &ReservationManager::batches() const
{
    return m_batches;
}

QString ReservationManager::batchForReservation(const QString &resId) const
{
    return m_resToBatchMap.value(resId);
}

QStringList ReservationManager::reservationsForBatch(const QString &batchId) const
{
    return m_batchToResMap.value(batchId);
}

void ReservationManager::removeBatch(const QString &batchId)
{
    // TODO make this atomic, ie. don't emit batch range notifications
    const auto res = m_batchToResMap.value(batchId);
    for (const auto &id : res) {
        if (id != batchId) {
            removeReservation(id);
        }
    }
    removeReservation(batchId);
}

void ReservationManager::loadBatches()
{
    Q_ASSERT(m_batches.empty());

    const auto base = batchesBasePath();
    if (!QDir::root().exists(base)) {
        initialBatchCreate();
        return;
    }

    QStringList batchesToRemove; // broken stuff detected during loading

    for (QDirIterator it(base, QDir::NoDotAndDotDot | QDir::Files); it.hasNext();) {
        it.next();
        QFile batchFile(it.filePath());
        if (!batchFile.open(QFile::ReadOnly)) {
            qCWarning(Log) << "Failed to open batch file" << it.filePath() << batchFile.errorString();
            continue;
        }

        const auto batchId = it.fileInfo().baseName();
        m_batches.push_back(batchId);

        const auto batchVal = JsonIO::read(batchFile.readAll());
        const auto top = batchVal.toObject();
        const auto resArray = top.value(QLatin1StringView("elements")).toArray();
        if (resArray.isEmpty()) {
            batchesToRemove.push_back(batchId);
            continue;
        }

        QStringList l;
        l.reserve(resArray.size());
        for (const auto &v : resArray) {
            const auto resId = v.toString();
            l.push_back(resId);
            m_resToBatchMap.insert(resId, batchId);
        }
        m_batchToResMap.insert(batchId, l);
    }

    std::sort(m_batches.begin(), m_batches.end(), [this](const auto &lhs, const auto &rhs) {
        return SortUtil::isBefore(reservation(lhs), reservation(rhs));
    });

    for (const auto &batchId : batchesToRemove) {
        storeRemoveBatch(batchId);
    }
}

void ReservationManager::storeBatch(const QString &batchId) const
{
    QJsonArray elems;
    const auto &batch = m_batchToResMap.value(batchId);
    std::copy(batch.begin(), batch.end(), std::back_inserter(elems));

    QJsonObject top;
    top.insert(QLatin1StringView("elements"), elems);

    const QString path = batchesBasePath() + batchId + QLatin1StringView(".json");
    QFile f(path);
    if (!f.open(QFile::WriteOnly | QFile::Truncate)) {
        qCWarning(Log) << "Failed to open batch file!" << path << f.errorString();
        return;
    }

    f.write(JsonIO::write(top));
}

void ReservationManager::storeRemoveBatch(const QString &batchId) const
{
    const QString path = batchesBasePath() + batchId + QLatin1StringView(".json");
    if (!QFile::remove(path)) {
        qCWarning(Log) << "Failed to remove file:" << path;
    }
}

void ReservationManager::initialBatchCreate()
{
    const auto batchBase = batchesBasePath();
    QDir::root().mkpath(batchBase);
    qCDebug(Log) << batchBase;

    const QSignalBlocker blocker(this);
    const auto base = reservationsBasePath();
    for (QDirIterator it(base, QDir::NoDotAndDotDot | QDir::Files); it.hasNext();) {
        it.next();
        const auto resId = it.fileInfo().baseName();
        const auto res = reservation(resId);
        updateBatch(resId, res, res);
    }
}

void ReservationManager::updateBatch(const QString &resId, const QVariant &newRes, const QVariant &oldRes)
{
    bool sortOrderInvalid = false;
    const auto oldBatchId = batchForReservation(resId);
    if (oldBatchId == resId) {
        const auto it = std::find(m_batches.begin(), m_batches.end(), resId);
        if (it != m_batches.begin() && it != m_batches.end()) {
            sortOrderInvalid |= SortUtil::startDateTime(reservation(*std::prev(it))) >= SortUtil::startDateTime(reservation(*it));
        }
        if (it != m_batches.end() && std::distance(it, m_batches.end()) > 1) {
            sortOrderInvalid |= SortUtil::startDateTime(reservation(*it)) >= SortUtil::startDateTime(reservation(*std::next(it)));
        }
        if (!sortOrderInvalid && m_batchToResMap.value(resId).size() == 1) {
            Q_EMIT batchContentChanged(resId);
            return;
        }
        if (sortOrderInvalid) { // otherwise the lower_bound search below doesn't work!
            removeFromBatch(resId, oldBatchId);
        }
    }
    QString newBatchId;

    // find the destination batch
    const auto beginIt = std::lower_bound(m_batches.begin(), m_batches.end(), newRes, [this](const auto &lhs, const auto &rhs) {
        return SortUtil::startDateTime(reservation(lhs)) < SortUtil::startDateTime(rhs);
    });
    for (auto it = beginIt; it != m_batches.end(); ++it) {
        const auto otherRes = (resId == (*it)) ? oldRes : reservation(*it);
        if (SortUtil::startDateTime(otherRes) != SortUtil::startDateTime(newRes)) {
            break; // no hit
        }
        if (MergeUtil::isSameIncidence(newRes, otherRes)) {
            newBatchId = *it;
            break;
        }
    }

    // still in the same batch?
    if (!oldBatchId.isEmpty() && oldBatchId == newBatchId) {
        Q_EMIT batchContentChanged(oldBatchId);
        // no need to store here, as batching didn't actually change
        return;
    }

    // move us out of the old batch
    // WARNING: beginIt will become invalid after this!
    if (!sortOrderInvalid) {
        removeFromBatch(resId, oldBatchId);
    }

    // insert us into the new batch
    if (newBatchId.isEmpty()) {
        // we are starting a new batch
        // re-run search for insertion point, could be invalid due to the above deletions
        const auto it = std::lower_bound(m_batches.begin(), m_batches.end(), newRes, [this](const auto &lhs, const auto &rhs) {
            return SortUtil::startDateTime(reservation(lhs)) < SortUtil::startDateTime(rhs);
        });
        m_batches.insert(it, QString(resId));
        m_batchToResMap.insert(resId, {resId});
        m_resToBatchMap.insert(resId, resId);
        Q_EMIT batchAdded(resId);
        storeBatch(resId);
    } else {
        m_batchToResMap[newBatchId].push_back(resId);
        m_resToBatchMap.insert(resId, newBatchId);
        Q_EMIT batchChanged(newBatchId);
        storeBatch(newBatchId);
    }
}

void ReservationManager::removeFromBatch(const QString &resId, const QString &batchId)
{
    if (batchId.isEmpty()) {
        return;
    }

    auto &batches = m_batchToResMap[batchId];
    m_resToBatchMap.remove(resId);
    if (batches.size() == 1) { // we were alone there, remove old batch
        m_batchToResMap.remove(batchId);
        const auto it = std::find(m_batches.begin(), m_batches.end(), batchId);
        m_batches.erase(it);
        Q_EMIT batchRemoved(batchId);
        storeRemoveBatch(batchId);
    } else if (resId == batchId) {
        // our id was the batch id, so rename the old batch
        batches.removeAll(resId);
        const QString renamedBatchId = batches.first();
        auto it = std::find(m_batches.begin(), m_batches.end(), batchId);
        Q_ASSERT(it != m_batches.end());
        *it = renamedBatchId;
        for (const auto &id : batches) {
            m_resToBatchMap[id] = renamedBatchId;
        }
        m_batchToResMap[renamedBatchId] = batches;
        m_batchToResMap.remove(batchId);
        Q_EMIT batchRenamed(batchId, renamedBatchId);
        storeRemoveBatch(batchId);
        storeBatch(renamedBatchId);
    } else {
        // old batch remains
        batches.removeAll(resId);
        Q_EMIT batchChanged(batchId);
        storeBatch(batchId);
    }
}

QVariant ReservationManager::isPartialUpdate(const QVariant &res) const
{
    // validate input
    if (!JsonLd::canConvert<Reservation>(res)) {
        return {};
    }
    const auto baseRes = JsonLd::convert<Reservation>(res);
    if (!baseRes.modifiedTime().isValid()) {
        return {};
    }

    // look for matching reservations in a 6 month window following the
    // modification time
    const auto rangeBegin = baseRes.modifiedTime();
    const auto rangeEnd = rangeBegin.addDays(6 * 30);

    const auto beginIt = std::lower_bound(m_batches.begin(), m_batches.end(), rangeBegin, [this](const auto &lhs, const auto &rhs) {
        return SortUtil::startDateTime(reservation(lhs)) < rhs;
    });
    for (auto it = beginIt; it != m_batches.end(); ++it) {
        auto otherRes = reservation(*it);
        if (SortUtil::startDateTime(otherRes) > rangeEnd) {
            break; // no hit
        }
        if (MergeUtil::isSame(res, otherRes)) {
            return otherRes;
        }
        if (MergeUtil::isSameIncidence(res, otherRes)) {
            // this is a multi-traveler element, check if we have it as one of the batch elements
            const auto &batch = m_batchToResMap.value(*it);
            for (const auto &batchedId : batch) {
                auto batchedRes = reservation(batchedId);
                if (MergeUtil::isSame(res, batchedRes)) {
                    return batchedRes;
                }
            }
        }
    }

    return {};
}

QList<QString> ReservationManager::applyPartialUpdate(const QVariant &res)
{
    // validate input
    if (!JsonLd::canConvert<Reservation>(res)) {
        return {};
    }
    const auto baseRes = JsonLd::convert<Reservation>(res);
    if (!baseRes.modifiedTime().isValid()) {
        return {};
    }

    // look for matching reservations in a 6 month window following the
    // modification time
    const auto rangeBegin = baseRes.modifiedTime();
    const auto rangeEnd = rangeBegin.addDays(6 * 30);

    QList<QString> updatedIds;
    const auto beginIt = std::lower_bound(m_batches.begin(), m_batches.end(), rangeBegin, [this](const auto &lhs, const auto &rhs) {
        return SortUtil::startDateTime(reservation(lhs)) < rhs;
    });
    for (auto it = beginIt; it != m_batches.end(); ++it) {
        const auto otherRes = reservation(*it);
        if (SortUtil::startDateTime(otherRes) > rangeEnd) {
            break; // no hit
        }
        if (MergeUtil::isSame(res, otherRes)) {
            // this is actually an update of otherRes!
            const auto newRes = MergeUtil::merge(otherRes, res);
            updateReservation(*it, newRes);
            updatedIds.push_back(*it);
            continue;
        }
        if (MergeUtil::isSameIncidence(res, otherRes)) {
            // this is a multi-traveler element, check if we have it as one of the
            // batch elements already
            const auto &batch = m_batchToResMap.value(*it);
            for (const auto &batchedId : batch) {
                const auto batchedRes = reservation(batchedId);
                if (MergeUtil::isSame(res, batchedRes)) {
                    // this is actually an update of a batched reservation
                    const auto newRes = MergeUtil::merge(otherRes, res);
                    updateReservation(batchedId, newRes);
                    updatedIds.push_back(batchedId);
                    break;
                }
            }
        }
    }

    return updatedIds;
}

QString ReservationManager::previousBatch(const QString &batchId) const
{
    // ### this can be optimized by relying on m_batches being sorted by start date
    const auto it = std::find(m_batches.begin(), m_batches.end(), batchId);
    if (it == m_batches.end() || it == m_batches.begin()) {
        return {};
    }
    return *(it - 1);
}

QString ReservationManager::nextBatch(const QString &batchId) const
{
    // ### this can be optimized by relying on m_batches being sorted by start date
    const auto it = std::find(m_batches.begin(), m_batches.end(), batchId);
    if (it == m_batches.end() || m_batches.size() < 2 || it == (m_batches.end() - 1)) {
        return {};
    }
    return *(it + 1);
}

QString ReservationManager::makeReservationId(const QString &resIdHint) const
{
    if (!resIdHint.isEmpty() && reservation(resIdHint).isNull()) {
        return resIdHint;
    }
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

#include "moc_reservationmanager.cpp"
