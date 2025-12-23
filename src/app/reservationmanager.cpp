/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reservationmanager.h"

#include "datetimehelper.h"
#include "json.h"
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

const QStringList& ReservationBatch::reservationIds() const
{
    return m_resIds;
}

QDateTime ReservationBatch::startDateTime() const
{
    return m_startDt;
}

QDateTime ReservationBatch::endDateTime() const
{
    return m_endDt;
}

bool ReservationBatch::isBefore(const ReservationBatch &lhs, const ReservationBatch &rhs)
{
    if (lhs.m_startDt == rhs.m_startDt) {
        return lhs.m_endDt < rhs.m_endDt;
    }
    return lhs.m_startDt < rhs.m_startDt;
}

QJsonObject ReservationBatch::toJson() const
{
    return Json::toJson(*this);
}

ReservationBatch ReservationBatch::fromJson(const QJsonObject &obj)
{
    return Json::fromJson<ReservationBatch>(obj);
}


bool ReservationManager::BatchComparator::operator()(const QString &lhs, const QString &rhs) const
{
    return ReservationBatch::isBefore(m_resMgr->m_batchToResMap.value(lhs), m_resMgr->m_batchToResMap.value(rhs));
}

bool ReservationManager::BatchComparator::operator()(const QString &lhs, const QDateTime &rhs) const
{
    return m_resMgr->m_batchToResMap.value(lhs).startDateTime() < rhs;
}

bool ReservationManager::BatchComparator::operator()(const QDateTime &lhs, const QString &rhs) const
{
    return lhs < m_resMgr->m_batchToResMap.value(rhs).startDateTime();
}


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

    loadReservationsInBatch(batchForReservation(id));
    return m_reservations.value(id);
}

void ReservationManager::loadReservation(const QString &resId) const
{
    const QString resPath = reservationsBasePath() + resId + ".jsonld"_L1;
    QFile f(resPath);
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to open JSON-LD reservation data file:" << resPath << f.errorString();
        return;
    }

    const auto val = JsonIO::read(f.readAll());
    if (!(val.isArray() && val.toArray().size() == 1) && !val.isObject()) {
        qCWarning(Log) << "Invalid JSON-LD reservation data file:" << resPath;
        return;
    }

    const auto resData = JsonLdDocument::fromJson(val.isArray() ? val.toArray() : QJsonArray({val.toObject()}));
    if (resData.size() != 1) {
        qCWarning(Log) << "Unable to parse JSON-LD reservation data file:" << resPath;
        return;
    }

    // re-run post-processing to benefit from newer augmentations
    ExtractorPostprocessor postproc;
    postproc.process(resData);
    if (postproc.result().size() != 1) {
        qCWarning(Log) << "Post-processing discarded the reservation:" << resPath;
        return;
    }

    const auto res = postproc.result().at(0);
    if (!m_validator.isValidElement(res)) {
        qCWarning(Log) << "Validation discarded the reservation:" << resPath;
        return;
    }
    m_reservations.insert(resId, res);
    qCDebug(Log) << "reservations loaded:" << m_reservations.size();
}

void ReservationManager::loadReservationsInBatch(const QString &batchId) const
{
    const auto &batch = m_batchToResMap.value(batchId);
    for (const auto &resId : batch.reservationIds()) {
        loadReservation(resId);
    }

    // cross-merge multi-traveler/multi-ticket incidences
    for (const auto &resId1 : batch.reservationIds()) {
        for (const auto &resId2 : batch.reservationIds()) {
            if (resId1 == resId2) {
                continue;
            }

            auto &res1 = m_reservations[resId1];
            auto &res2 = m_reservations[resId2];
            res1 = MergeUtil::mergeIncidence(res1, res2);
            res2 = MergeUtil::mergeIncidence(res2, res1);
        }
    }
}

QString ReservationManager::reservationsBasePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/reservations/"_L1;
}

QString ReservationManager::batchesBasePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/batches/"_L1;
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

        if (const auto resId = addReservation(res); !resId.isEmpty()) {
            ids.push_back(resId);
        }
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

    const auto beginIt = std::lower_bound(m_batches.begin(), m_batches.end(), rangeBegin, BatchComparator(this));
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
            auto &batch = m_batchToResMap[*it];
            for (const auto &batchedId : batch.m_resIds) {
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

            batch.m_resIds.push_back(resId);
            populateBatchTimes(batch);
            m_resToBatchMap.insert(resId, *it);
            storeBatch(*it, batch);
            Q_EMIT batchChanged(*it);
            return resId;
        }
    }

    // truly new, and starting a new batch
    const QString resId = makeReservationId(resIdHint);
    storeReservation(resId, res);
    Q_EMIT reservationAdded(resId);

    // search for the precise insertion place, beginIt is only the begin of our initial search range
    const auto insertIt = std::lower_bound(m_batches.begin(), m_batches.end(), SortUtil::startDateTime(res), BatchComparator(this));
    m_batches.insert(insertIt, resId);
    ReservationBatch batch;
    batch.m_resIds = {resId};
    populateBatchTimes(batch);
    m_batchToResMap.insert(resId, batch);
    m_resToBatchMap.insert(resId, resId);
    storeBatch(resId, batch);
    Q_EMIT batchAdded(resId);
    return resId;
}

QString ReservationManager::addReservationWithPostProcessing(const QVariant &resData)
{
    const auto l = addReservationsWithPostProcessing({resData});
    return !l.isEmpty() ? l.at(0) : QString();
}

void ReservationManager::updateReservation(const QString &resId, QVariant res)
{
    const auto oldRes = reservation(resId);

    ExtractorPostprocessor postproc;
    postproc.process({res});
    const auto postProcResult = postproc.result();
    if (postProcResult.size() == 1) {
        res = postProcResult.at(0);
    }

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

void ReservationManager::updateBatch(const std::vector<ReservationManager::ReservationChange> &changeset)
{
    if (changeset.empty()) {
        return;
    }

    QString batchId;
    for (const auto &change : changeset) {
        const auto bid = batchForReservation(change.id);
        if (batchId.isEmpty()) {
            batchId = bid;
        }
        assert(batchId == bid && !batchId.isEmpty());
        storeReservation(change.id, change.res);
    }

    auto &batch = m_batchToResMap[batchId];
    populateBatchTimes(m_batchToResMap[batchId]);
    storeBatch(batchId, batch);

    // TODO can probably be done more efficiently by only moving batchId
    std::ranges::sort(m_batches, BatchComparator(this));
    Q_EMIT batchContentChanged(batchId);
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
    return m_batchToResMap.value(batchId).reservationIds();
}

void ReservationManager::removeBatch(const QString &batchId)
{
    // TODO make this atomic, ie. don't emit batch range notifications
    const auto batch = m_batchToResMap.value(batchId);
    for (const auto &id : batch.reservationIds()) {
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

        const auto batch = ReservationBatch::fromJson(JsonIO::read(batchFile.readAll()).toObject());
        if (batch.reservationIds().isEmpty()) {
            batchesToRemove.push_back(batchId);
            continue;
        }

        for (const auto &resId : batch.reservationIds()) {
            m_resToBatchMap.insert(resId, batchId);
        }
        m_batchToResMap.insert(batchId, batch);
    }

    // populate batch times where they are missing, e.g. for legacy app data
    for (auto it = m_batchToResMap.begin(); it != m_batchToResMap.end(); ++it) {
        if (!it.value().startDateTime().isValid()) {
            populateBatchTimes(it.value());
            storeBatch(it.key(), it.value());
        }
    }

    std::ranges::sort(m_batches, BatchComparator(this));

    for (const auto &batchId : batchesToRemove) {
        storeRemoveBatch(batchId);
    }
}

void ReservationManager::storeBatch(const QString &batchId, const ReservationBatch &batch)
{
    const QString path = batchesBasePath() + batchId + ".json"_L1;
    QFile f(path);
    if (!f.open(QFile::WriteOnly | QFile::Truncate)) {
        qCWarning(Log) << "Failed to open batch file!" << path << f.errorString();
        return;
    }

    f.write(JsonIO::write(batch.toJson()));
}

void ReservationManager::storeRemoveBatch(const QString &batchId)
{
    const QString path = batchesBasePath() + batchId + ".json"_L1;
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

void ReservationManager::populateBatchTimes(ReservationBatch &batch) const
{
    for (const auto &resId : batch.reservationIds()) {
        batch.m_startDt = SortUtil::startDateTime(reservation(resId));
        batch.m_endDt = SortUtil::endDateTime(reservation(resId));
        if (batch.m_startDt.isValid() && batch.m_endDt.isValid()) {
            break;
        }
    }
}

// same as ReservationBatch::isBefore, but for use before batch times have been updated
[[nodiscard]] static bool reservationIsBefore(const QVariant &lhs, const QVariant &rhs)
{
    const auto lhsStart = SortUtil::startDateTime(lhs);
    const auto rhsStart = SortUtil::startDateTime(rhs);
    if (lhsStart == rhsStart) {
        return SortUtil::endDateTime(lhs) > SortUtil::endDateTime(rhs);
    }
    return lhsStart < rhsStart;
}

void ReservationManager::updateBatch(const QString &resId, const QVariant &newRes, const QVariant &oldRes)
{
    bool sortOrderInvalid = false;
    const auto oldBatchId = batchForReservation(resId);
    if (oldBatchId == resId) {
        const auto it = std::find(m_batches.begin(), m_batches.end(), resId);
        if (it != m_batches.begin() && it != m_batches.end()) {
            sortOrderInvalid |= reservationIsBefore(reservation(*it), reservation(*std::prev(it)));
        }
        if (it != m_batches.end() && std::distance(it, m_batches.end()) > 1) {
            sortOrderInvalid |= reservationIsBefore(reservation(*std::next(it)), reservation(*it));
        }
        if (sortOrderInvalid) { // otherwise the lower_bound search below doesn't work!
            removeFromBatch(resId, oldBatchId);
        }
    }
    QString newBatchId;

    // find the destination batch
    const auto beginIt = std::lower_bound(m_batches.begin(), m_batches.end(), SortUtil::startDateTime(newRes), BatchComparator(this));
    for (auto it = beginIt; it != m_batches.end(); ++it) {
        const auto otherRes = (resId == (*it)) ? oldRes : reservation(*it);
        if (!DateTimeHelper::isSameDateTime(SortUtil::startDateTime(otherRes), SortUtil::startDateTime(newRes))) {
            break; // no hit
        }
        if (MergeUtil::isSameIncidence(newRes, otherRes)) {
            newBatchId = *it;
            break;
        }
    }

    // still in the same batch?
    if (!oldBatchId.isEmpty() && oldBatchId == newBatchId) {
        auto &batch = m_batchToResMap[newBatchId];
        populateBatchTimes(batch);
        storeBatch(newBatchId, batch);
        Q_EMIT batchContentChanged(oldBatchId);
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
        const auto it = std::lower_bound(m_batches.begin(), m_batches.end(), SortUtil::startDateTime(newRes), BatchComparator(this));
        m_batches.insert(it, QString(resId));
        ReservationBatch batch;
        batch.m_resIds = {resId};
        populateBatchTimes(batch);
        m_batchToResMap.insert(resId, batch);
        m_resToBatchMap.insert(resId, resId);
        Q_EMIT batchAdded(resId);
        storeBatch(resId, batch);
    } else {
        auto &batch = m_batchToResMap[newBatchId];
        batch.m_resIds.push_back(resId);
        populateBatchTimes(batch);
        m_resToBatchMap.insert(resId, newBatchId);
        Q_EMIT batchChanged(newBatchId);
        storeBatch(newBatchId, batch);
    }
}

void ReservationManager::removeFromBatch(const QString &resId, const QString &batchId)
{
    if (batchId.isEmpty()) {
        return;
    }

    auto &batch = m_batchToResMap[batchId];
    m_resToBatchMap.remove(resId);
    if (batch.reservationIds().size() == 1) { // we were alone there, remove old batch
        m_batchToResMap.remove(batchId);
        const auto it = std::find(m_batches.begin(), m_batches.end(), batchId);
        m_batches.erase(it);
        Q_EMIT batchRemoved(batchId);
        storeRemoveBatch(batchId);
    } else if (resId == batchId) {
        // our id was the batch id, so rename the old batch
        batch.m_resIds.removeAll(resId);
        const QString renamedBatchId = batch.reservationIds().first();
        auto it = std::find(m_batches.begin(), m_batches.end(), batchId);
        Q_ASSERT(it != m_batches.end());
        *it = renamedBatchId;
        for (const auto &id : batch.reservationIds()) {
            m_resToBatchMap[id] = renamedBatchId;
        }
        m_batchToResMap[renamedBatchId] = batch;
        m_batchToResMap.remove(batchId);
        Q_EMIT batchRenamed(batchId, renamedBatchId);
        storeRemoveBatch(batchId);
        storeBatch(renamedBatchId, batch);
    } else {
        // old batch remains
        batch.m_resIds.removeAll(resId);
        Q_EMIT batchChanged(batchId);
        storeBatch(batchId, batch);
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

    const auto beginIt = std::lower_bound(m_batches.begin(), m_batches.end(), rangeBegin, BatchComparator(this));
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
            for (const auto &batchedId : batch.reservationIds()) {
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
    const auto beginIt = std::lower_bound(m_batches.begin(), m_batches.end(), rangeBegin, BatchComparator(this));
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
            for (const auto &batchedId : batch.reservationIds()) {
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

ReservationBatch ReservationManager::batch(const QString &batchId) const
{
    return m_batchToResMap.value(batchId);
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
