/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reservationmanager.h"
#include "pkpassmanager.h"
#include "logging.h"

#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/MergeUtil>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>
#include <KItinerary/Visit>

#include <KPkPass/Pass>

#ifdef Q_OS_ANDROID
#include <KMime/Message>
#endif

#include <KLocalizedString>

#include <QDate>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScopeGuard>
#include <QStandardPaths>
#include <QUrl>
#include <QUuid>
#include <QVector>

using namespace KItinerary;

static bool isSameTrip(const QVariant &lhs, const QVariant &rhs)
{
    if (lhs.userType() != rhs.userType() || !JsonLd::canConvert<Reservation>(lhs) || !JsonLd::canConvert<Reservation>(rhs)) {
        return false;
    }

    const auto lhsTrip = JsonLd::convert<Reservation>(lhs).reservationFor();
    const auto rhsTrip = JsonLd::convert<Reservation>(rhs).reservationFor();
    return MergeUtil::isSame(lhsTrip, rhsTrip);
}

ReservationManager::ReservationManager(QObject* parent)
    : QObject(parent)
{
    m_validator.setAcceptedTypes<
        BusReservation,
        EventReservation,
        FlightReservation,
        FoodEstablishmentReservation,
        LodgingReservation,
        RentalCarReservation,
        TrainReservation,
        TouristAttractionVisit
    >();
    m_validator.setAcceptOnlyCompleteElements(true);

    loadBatches();
}

ReservationManager::~ReservationManager() = default;

void ReservationManager::setPkPassManager(PkPassManager* mgr)
{
    m_passMgr = mgr;
    connect(mgr, &PkPassManager::passAdded, this, &ReservationManager::passAdded);
    connect(mgr, &PkPassManager::passUpdated, this, &ReservationManager::passUpdated);
    connect(mgr, &PkPassManager::passRemoved, this, &ReservationManager::passRemoved);
}

bool ReservationManager::isEmpty() const
{
    return m_batchToResMap.empty();
}

bool ReservationManager::hasBatch(const QString &batchId) const
{
    return m_batchToResMap.contains(batchId);
}

QVariant ReservationManager::reservation(const QString& id) const
{
    if (id.isEmpty()) {
        return {};
    }

    const auto it = m_reservations.constFind(id);
    if (it != m_reservations.constEnd()) {
        return it.value();
    }

    const QString resPath = reservationsBasePath() + id + QLatin1String(".jsonld");
    QFile f(resPath);
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to open JSON-LD reservation data file:" << resPath << f.errorString();
        return {};
    }

    const auto doc = QJsonDocument::fromJson(f.readAll());
    if (!(doc.isArray() && doc.array().size() == 1) && !doc.isObject()) {
        qCWarning(Log) << "Invalid JSON-LD reservation data file:" << resPath;
        return {};
    }

    const auto resData = JsonLdDocument::fromJson(doc.isArray() ? doc.array() : QJsonArray({doc.object()}));
    if (resData.size() != 1) {
        qCWarning(Log) << "Unable to parse JSON-LD reservation data file:" << resPath;
        return {};
    }

    // re-run post-processing to benefit from newer augmentations
    ExtractorPostprocessor postproc;
    postproc.setValidationEnabled(false);
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
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/reservations/");
}

QString ReservationManager::batchesBasePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/batches/");
}

QVector<QString> ReservationManager::importReservation(const QByteArray& data, const QString &fileName)
{
    ExtractorEngine engine;
    engine.setContextDate(QDateTime(QDate::currentDate(), QTime(0, 0)));
    engine.setData(data, fileName);
    return importReservations(JsonLdDocument::fromJson(engine.extract()));
}

#ifdef Q_OS_ANDROID
QVector<QString> ReservationManager::importReservation(KMime::Message *msg)
{
    ExtractorEngine engine;
    engine.setContent(QVariant::fromValue<KMime::Content*>(msg), u"message/rfc822");
    return importReservations(JsonLdDocument::fromJson(engine.extract()));
}
#endif

QVector<QString> ReservationManager::importReservations(const QVector<QVariant> &resData)
{
    ExtractorPostprocessor postproc;
    postproc.setValidationEnabled(false);
    postproc.setContextDate(QDateTime(QDate::currentDate(), QTime(0, 0)));
    postproc.process(resData);

    auto data = postproc.result();
    QVector<QString> ids;
    ids.reserve(data.size());
    for (auto &res : data) {
        if (JsonLd::isA<Event>(res)) { // promote Event to EventReservation
            EventReservation ev;
            ev.setReservationFor(res);
            res = ev;
        }
        // TODO show UI asking for time ranges for LodgingBusiness, FoodEstablishment, etc

        // filter out non-Reservation objects we can't handle yet
        if (!m_validator.isValidElement(res)) {

            // check if this is a minimal cancellation element
            const auto cleanup = qScopeGuard([this]{ m_validator.setAcceptOnlyCompleteElements(true); });
            m_validator.setAcceptOnlyCompleteElements(false);
            if (m_validator.isValidElement(res)) {
                ids += applyPartialUpdate(res);
                continue;
            }

            qCWarning(Log) << "Discarding imported element due to validation failure" << res;
            continue;
        }

        ids.push_back(addReservation(res));
    }

    Q_EMIT infoMessage(i18np("One reservation imported.", "%1 reservations imported.", ids.size()));
    return ids;
}

QString ReservationManager::addReservation(const QVariant &res)
{
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
        if (isSameTrip(res, otherRes)) {
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
            const QString resId = QUuid::createUuid().toString();
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
    const QString resId = QUuid::createUuid().toString();
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
    const QString path = basePath + resId + QLatin1String(".jsonld");
    QFile f(path);
    if (!f.open(QFile::WriteOnly)) {
        qCWarning(Log) << "Unable to open file:" << f.errorString();
        return;
    }
    f.write(QJsonDocument(JsonLdDocument::toJson(res)).toJson());
    m_reservations.insert(resId, res);
}

void ReservationManager::removeReservation(const QString& id)
{
    const auto batchId = m_resToBatchMap.value(id);
    removeFromBatch(id, batchId);

    const QString basePath = reservationsBasePath();
    QFile::remove(basePath + QLatin1Char('/') + id + QLatin1String(".jsonld"));
    Q_EMIT reservationRemoved(id);
    m_reservations.remove(id);
}

const std::vector<QString>& ReservationManager::batches() const
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

void ReservationManager::passAdded(const QString& passId)
{
    const auto pass = m_passMgr->pass(passId);
    ExtractorEngine engine;
    engine.setContent(QVariant::fromValue<KPkPass::Pass*>(pass), u"application/vnd.apple.pkpass");
    const auto data = engine.extract();
    const auto res = JsonLdDocument::fromJson(data);
    importReservations(res);
}

void ReservationManager::passUpdated(const QString& passId)
{
    passAdded(passId);
}

void ReservationManager::passRemoved(const QString& passId)
{
    Q_UNUSED(passId)
    // TODO
}

void ReservationManager::loadBatches()
{
    Q_ASSERT(m_batches.empty());

    const auto base = batchesBasePath();
    if (!QDir::root().exists(base)) {
        initialBatchCreate();
        return;
    }

    for (QDirIterator it(base, QDir::NoDotAndDotDot | QDir::Files); it.hasNext();) {
        it.next();
        QFile batchFile(it.filePath());
        if (!batchFile.open(QFile::ReadOnly)) {
            qCWarning(Log) << "Failed to open batch file" << it.filePath() << batchFile.errorString();
            continue;
        }

        const auto batchId = it.fileInfo().baseName();
        m_batches.push_back(batchId);

        const auto batchDoc = QJsonDocument::fromJson(batchFile.readAll());
        const auto top = batchDoc.object();
        const auto resArray = top.value(QLatin1String("elements")).toArray();
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
}

void ReservationManager::storeBatch(const QString &batchId) const
{
    QJsonArray elems;
    const auto &batch = m_batchToResMap.value(batchId);
    std::copy(batch.begin(), batch.end(), std::back_inserter(elems));

    QJsonObject top;
    top.insert(QLatin1String("elements"), elems);

    const QString path = batchesBasePath() + batchId + QLatin1String(".json");
    QFile f(path);
    if (!f.open(QFile::WriteOnly | QFile::Truncate)) {
        qCWarning(Log) << "Failed to open batch file!" << path << f.errorString();
        return;
    }

    f.write(QJsonDocument(top).toJson());
}

void ReservationManager::storeRemoveBatch(const QString &batchId) const
{
    const QString path = batchesBasePath() + batchId + QLatin1String(".json");
    QFile::remove(path);
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
    const auto oldBatchId = batchForReservation(resId);
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
        if (isSameTrip(newRes, otherRes)) {
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
    removeFromBatch(resId, oldBatchId);

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

QVector<QString> ReservationManager::applyPartialUpdate(const QVariant &res)
{
    // validate input
    if (!JsonLd::canConvert<Reservation>(res)) {
        return {};
    }
    const auto baseRes = JsonLd::convert<Reservation>(res);
    if (!baseRes.modifiedTime().isValid()) {
        return {};
    }

    // look for matching reservations in a 6 month window following the modification time
    const auto rangeBegin = baseRes.modifiedTime();
    const auto rangeEnd = rangeBegin.addDays(6 * 30);

    QVector<QString> updatedIds;
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
        if (isSameTrip(res, otherRes)) {
            // this is a multi-traveler element, check if we have it as one of the batch elements already
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

QString ReservationManager::nextBatch(const QString& batchId) const
{
    // ### this can be optimized by relying on m_batches being sorted by start date
    const auto it = std::find(m_batches.begin(), m_batches.end(), batchId);
    if (it == m_batches.end() || m_batches.size() < 2 || it == (m_batches.end() - 1)) {
        return {};
    }
    return *(it + 1);
}
