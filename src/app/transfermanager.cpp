/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "transfermanager.h"
#include "logging.h"
#include "favoritelocationmodel.h"
#include "publictransport.h"
#include "reservationmanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>

#include <KPublicTransport/Journey>
#include <KPublicTransport/JourneyReply>
#include <KPublicTransport/Manager>

#include <KLocalizedString>

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QStandardPaths>

using namespace KItinerary;

// bump this to trigger a full rescan for transfers
enum { CurrentFullScanVerion = 1 };

TransferManager::TransferManager(QObject *parent)
    : QObject(parent)
{
}

TransferManager::~TransferManager() = default;

void TransferManager::setReservationManager(ReservationManager *resMgr)
{
    m_resMgr = resMgr;
    connect(m_resMgr, &ReservationManager::batchAdded, this, qOverload<const QString&>(&TransferManager::checkReservation));
    connect(m_resMgr, &ReservationManager::batchChanged, this, qOverload<const QString&>(&TransferManager::checkReservation));
    connect(m_resMgr, &ReservationManager::batchRemoved, this, &TransferManager::reservationRemoved);
    rescan();
}

void TransferManager::setTripGroupManager(TripGroupManager* tgMgr)
{
    m_tgMgr = tgMgr;
    connect(m_tgMgr, &TripGroupManager::tripGroupAdded, this, &TransferManager::tripGroupChanged);
    connect(m_tgMgr, &TripGroupManager::tripGroupChanged, this, &TransferManager::tripGroupChanged);
    rescan();
}

void TransferManager::setFavoriteLocationModel(FavoriteLocationModel *favLocModel)
{
    m_favLocModel = favLocModel;
    connect(m_favLocModel, &FavoriteLocationModel::rowsInserted, this, [this]() { rescan(true); });
    rescan();
}

void TransferManager::setPublicTransportManager(KPublicTransport::Manager *ptMgr)
{
    m_ptrMgr = ptMgr;
}

void TransferManager::setAutoAddTransfers(bool enable)
{
    m_autoAddTransfers = enable;
    rescan();
}

void TransferManager::setAutoFillTransfers(bool enable)
{
    m_autoFillTransfers = enable;
}

Transfer TransferManager::transfer(const QString &resId, Transfer::Alignment alignment) const
{
    const auto it = m_transfers[alignment].constFind(resId);
    if (it != m_transfers[alignment].constEnd()) {
        return it.value();
    }

    const auto t = readFromFile(resId, alignment);
    m_transfers[alignment].insert(resId, t);
    return t;
}

void TransferManager::setJourneyForTransfer(Transfer transfer, const KPublicTransport::Journey &journey)
{
    transfer.setState(Transfer::Selected);
    transfer.setJourney(journey);
    m_transfers[transfer.alignment()].insert(transfer.reservationId(), transfer);
    writeToFile(transfer);
    emit transferChanged(transfer);
}

Transfer TransferManager::setFavoriteLocationForTransfer(Transfer transfer, const FavoriteLocation& favoriteLocation)
{
    if (transfer.floatingLocationType() != Transfer::FavoriteLocation) {
        qCWarning(Log) << "Attempting to changing transfer floating location of wrong type";
        return transfer;
    }

    KPublicTransport::Location loc;
    loc.setLatitude(favoriteLocation.latitude());
    loc.setLongitude(favoriteLocation.longitude());

    if (transfer.alignment() == Transfer::Before) {
        transfer.setFrom(loc);
        transfer.setFromName(favoriteLocation.name());
    } else {
        transfer.setTo(loc);
        transfer.setToName(favoriteLocation.name());
    }
    transfer.setJourney({});
    m_transfers[transfer.alignment()].insert(transfer.reservationId(), transfer);
    writeToFile(transfer);
    emit transferChanged(transfer);

    return transfer;
}

void TransferManager::discardTransfer(Transfer transfer)
{
    transfer.setState(Transfer::Discarded);
    transfer.setJourney({});
    addOrUpdateTransfer(transfer);
}

bool TransferManager::canAddTransfer(const QString& resId, Transfer::Alignment alignment) const
{
    auto t = transfer(resId, alignment);
    if (t.state() == Transfer::Selected || t.state() == Transfer::Pending) {
        return false; // already exists
    }

    const auto res = m_resMgr->reservation(resId);
    // in case it's new
    t.setReservationId(resId);
    t.setAlignment(alignment);

    return (alignment == Transfer::Before ? checkTransferBefore(resId, res, t) : checkTransferAfter(resId, res, t)) != ShouldRemove;
}

Transfer TransferManager::addTransfer(const QString& resId, Transfer::Alignment alignment)
{
    const auto res = m_resMgr->reservation(resId);

    auto t = transfer(resId, alignment);
    // in case this is new
    t.setReservationId(resId);
    t.setAlignment(alignment);
    // in case this was previously discarded
    t.setState(Transfer::UndefinedState);
    determineAnchorDeltaDefault(t, res);

    if ((alignment == Transfer::Before ? checkTransferBefore(resId, res, t) : checkTransferAfter(resId, res, t)) != ShouldRemove) {
        addOrUpdateTransfer(t);
        return t;
    } else {
        return {};
    }
}

void TransferManager::rescan(bool force)
{
    if (!m_resMgr || !m_tgMgr || !m_favLocModel || !m_autoAddTransfers) {
        return;
    }

    QSettings settings;
    settings.beginGroup(QStringLiteral("TransferManager"));
    const auto previousFullScanVersion = settings.value(QLatin1String("FullScan"), 0).toInt();
    if (!force && previousFullScanVersion >= CurrentFullScanVerion) {
        return;
    }

    qCInfo(Log) << "Performing a full transfer search..." << previousFullScanVersion;
    for (const auto &batchId : m_resMgr->batches()) {
        checkReservation(batchId);
    }
    settings.setValue(QStringLiteral("FullScan"), CurrentFullScanVerion);
}

void TransferManager::checkReservation(const QString &resId)
{
    if (!m_autoAddTransfers) {
        return;
    }

    const auto res = m_resMgr->reservation(resId);

    const auto now = currentDateTime();
    if (SortUtil::endDateTime(res) < now) {
        return;
    }
    checkReservation(resId, res, Transfer::After);
    if (SortUtil::startDateTime(res) < now) {
        return;
    }
    checkReservation(resId, res, Transfer::Before);
}

void TransferManager::checkReservation(const QString &resId, const QVariant &res, Transfer::Alignment alignment)
{
    auto t = transfer(resId, alignment);
    if (t.state() == Transfer::Discarded) { // user already discarded this
        return;
    }

    // in case this is new
    t.setReservationId(resId);
    t.setAlignment(alignment);
    determineAnchorDeltaDefault(t, res);

    const auto action = alignment == Transfer::Before ? checkTransferBefore(resId, res, t) : checkTransferAfter(resId, res, t);
    switch (action) {
        case ShouldAutoAdd:
            addOrUpdateTransfer(t);
            break;
        case CanAddManually:
            break;
        case ShouldRemove:
            removeTransfer(t);
            break;
    }
}

TransferManager::CheckTransferResult TransferManager::checkTransferBefore(const QString &resId, const QVariant &res, Transfer &transfer) const
{
    transfer.setAnchorTime(SortUtil::startDateTime(res));
    const auto isLocationChange = LocationUtil::isLocationChange(res);
    QVariant toLoc;
    if (isLocationChange) {
        toLoc = LocationUtil::departureLocation(res);
    } else {
        toLoc = LocationUtil::location(res);
    }
    transfer.setTo(PublicTransport::locationFromPlace(toLoc, res));
    transfer.setToName(LocationUtil::name(toLoc));

    // TODO pre-transfers should happen in the following cases:
    // - res is a location change and we are currently at home (== first element in a trip group)
    // - res is a location change and we are not at the departure location yet
    // - res is an event and we are not at its location already
    // ... and can happen in the following cases:
    // - res is not in a trip group at all (that assumes we are at home)

    const auto notInGroup = isNotInTripGroup(resId);
    if ((isLocationChange && isFirstInTripGroup(resId)) || notInGroup) {
        const auto f = pickFavorite(toLoc, resId, Transfer::Before);
        transfer.setFrom(locationFromFavorite(f));
        transfer.setFromName(f.name());
        transfer.setFloatingLocationType(Transfer::FavoriteLocation);
        return notInGroup ? CanAddManually : ShouldAutoAdd;
    }

    if (isLocationChange) {
        const auto prevResId = m_resMgr->previousBatch(resId); // TODO this fails for multiple nested range elements!
        if (prevResId.isEmpty()) {
            return ShouldRemove;
        }
        const auto prevRes = m_resMgr->reservation(prevResId);
        // TODO this needs to consider transfers before nextResId
        QVariant prevLoc;
        if (LocationUtil::isLocationChange(prevRes)) {
            prevLoc = LocationUtil::arrivalLocation(prevRes);
        } else {
            prevLoc = LocationUtil::location(prevRes);
        }
        if (!toLoc.isNull() && !prevLoc.isNull() && !LocationUtil::isSameLocation(toLoc, prevLoc, LocationUtil::WalkingDistance)) {
            qDebug() << res << prevRes << LocationUtil::name(toLoc) << LocationUtil::name(prevLoc);
            transfer.setFrom(PublicTransport::locationFromPlace(prevLoc, prevRes));
            transfer.setFromName(LocationUtil::name(prevLoc));
            return ShouldAutoAdd;

        }
    }

    // TODO

    return ShouldRemove;
}

TransferManager::CheckTransferResult TransferManager::checkTransferAfter(const QString &resId, const QVariant &res, Transfer &transfer) const
{
    transfer.setAnchorTime(SortUtil::endDateTime(res));
    const auto isLocationChange = LocationUtil::isLocationChange(res);
    QVariant fromLoc;
    if (isLocationChange) {
        fromLoc = LocationUtil::arrivalLocation(res);
    } else {
        fromLoc = LocationUtil::location(res);
    }
    transfer.setFrom(PublicTransport::locationFromPlace(fromLoc, res));
    transfer.setFromName(LocationUtil::name(fromLoc));

    // TODO post-transfer should happen in the following cases:
    // - res is a location change and we are the last element in a trip group (ie. going home)
    // - res is a location change and the following element is in a different location, or has a different departure location
    // - res is an event and the following or enclosing element is a lodging element
    // ... and can happen in the following cases
    // - res is not in a trip group at all (that assumes we are at home)

    const auto notInGroup = isNotInTripGroup(resId);
    if ((isLocationChange && isLastInTripGroup(resId)) || notInGroup) {
        const auto f = pickFavorite(fromLoc, resId, Transfer::After);
        transfer.setTo(locationFromFavorite(f));
        transfer.setToName(f.name());
        transfer.setToName(i18n("Home"));
        transfer.setFloatingLocationType(Transfer::FavoriteLocation);
        return notInGroup ? CanAddManually : ShouldAutoAdd;
    }

    if (isLocationChange) {
        const auto nextResId = m_resMgr->nextBatch(resId);
        if (nextResId.isEmpty()) {
            return ShouldRemove;
        }
        // TODO this needs to consider transfers after nextResId
        const auto nextRes = m_resMgr->reservation(nextResId);
        QVariant nextLoc;
        if (LocationUtil::isLocationChange(nextRes)) {
            nextLoc = LocationUtil::departureLocation(nextRes);
        } else {
            nextLoc = LocationUtil::location(nextRes);
        }
        if (!fromLoc.isNull() && !nextLoc.isNull() && !LocationUtil::isSameLocation(fromLoc, nextLoc, LocationUtil::WalkingDistance)) {
            qDebug() << res << nextRes << LocationUtil::name(fromLoc) << LocationUtil::name(nextLoc);
            transfer.setTo(PublicTransport::locationFromPlace(nextLoc, nextRes));
            transfer.setToName(LocationUtil::name(nextLoc));
            return ShouldAutoAdd;
        }
    }

    // TODO

    return ShouldRemove;
}

void TransferManager::reservationRemoved(const QString &resId)
{
    m_transfers[Transfer::Before].remove(resId);
    m_transfers[Transfer::After].remove(resId);
    removeFile(resId, Transfer::Before);
    removeFile(resId, Transfer::After);
    // TODO updates to adjacent transfers?
    emit transferRemoved(resId, Transfer::Before);
    emit transferRemoved(resId, Transfer::After);
}

void TransferManager::tripGroupChanged(const QString &tgId)
{
    const auto tg = m_tgMgr->tripGroup(tgId);
    for (const auto &resId : tg.elements()) {
        checkReservation(resId);
    }
}

bool TransferManager::isFirstInTripGroup(const QString &resId) const
{
    const auto tgId = m_tgMgr->tripGroupForReservation(resId);
    return tgId.elements().empty() ? false : tgId.elements().at(0) == resId;
}

bool TransferManager::isLastInTripGroup(const QString &resId) const
{
    const auto tgId = m_tgMgr->tripGroupForReservation(resId);
    return tgId.elements().empty() ? false : tgId.elements().constLast() == resId;
}

bool TransferManager::isNotInTripGroup(const QString &resId) const
{
    return m_tgMgr->tripGroupIdForReservation(resId).isEmpty();
}

void TransferManager::determineAnchorDeltaDefault(Transfer &transfer, const QVariant &res) const
{
    if (transfer.state() != Transfer::UndefinedState) {
        return;
    }

    if (JsonLd::isA<FlightReservation>(res)) {
        transfer.setAnchorTimeDelta(transfer.alignment() == Transfer::Before ? 60 * 60 : 30 * 60);
    } else {
        transfer.setAnchorTimeDelta(10 * 60);
    }
}

KPublicTransport::Location TransferManager::locationFromFavorite(const FavoriteLocation &favLoc)
{
    KPublicTransport::Location loc;
    loc.setLatitude(favLoc.latitude());
    loc.setLongitude(favLoc.longitude());
    return loc;
}

FavoriteLocation TransferManager::pickFavorite(const QVariant &anchoredLoc, const QString &resId, Transfer::Alignment alignment) const
{
    // TODO selection strategy:
    // (1) pick the same favorite as was used before/after resId
    // (2) pick the favorite closest to anchoredLoc - this can work very well if the favorites aren't close to each other
    // (3) pick the first one

    Q_UNUSED(anchoredLoc)
    Q_UNUSED(resId)
    Q_UNUSED(alignment)

    if (m_favLocModel->rowCount() == 0) {
        return {};
    }
    return m_favLocModel->favoriteLocations()[0];
}

void TransferManager::addOrUpdateTransfer(Transfer &t)
{
    if (t.state() == Transfer::UndefinedState) { // newly added
        if (!t.hasLocations()) { // undefined home location
            return;
        }
        t.setState(Transfer::Pending);
        autoFillTransfer(t);
        m_transfers[t.alignment()].insert(t.reservationId(), t);
        writeToFile(t);
        emit transferAdded(t);
    } else if (t.state() == Transfer::Discarded) {
        m_transfers[t.alignment()].insert(t.reservationId(), t);
        writeToFile(t);
        emit transferRemoved(t.reservationId(), t.alignment());
    } else { // update existing data
        m_transfers[t.alignment()].insert(t.reservationId(), t);
        writeToFile(t);
        emit transferChanged(t);
    }
}

void TransferManager::removeTransfer(const Transfer &t)
{
    if (t.state() == Transfer::UndefinedState) { // this was never added
        return;
    }
    m_transfers[t.alignment()].remove(t.reservationId());
    removeFile(t.reservationId(), t.alignment());
    emit transferRemoved(t.reservationId(), t.alignment());
}

static QString transferBasePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/transfers/");
}

Transfer TransferManager::readFromFile(const QString& resId, Transfer::Alignment alignment) const
{
    const QString fileName = transferBasePath() + Transfer::identifier(resId, alignment) + QLatin1String(".json");
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly)) {
        return {};
    }
    return Transfer::fromJson(QJsonDocument::fromJson(f.readAll()).object());
}

void TransferManager::writeToFile(const Transfer &transfer) const
{
    QDir().mkpath(transferBasePath());
    const QString fileName = transferBasePath() + transfer.identifier() + QLatin1String(".json");
    QFile f(fileName);
    if (!f.open(QFile::WriteOnly)) {
        qCWarning(Log) << "Failed to store transfer data" << f.fileName() << f.errorString();
        return;
    }
    f.write(QJsonDocument(Transfer::toJson(transfer)).toJson());
}

void TransferManager::removeFile(const QString &resId, Transfer::Alignment alignment) const
{
    const QString fileName = transferBasePath() + Transfer::identifier(resId, alignment) + QLatin1String(".json");
    QFile::remove(fileName);
}

void TransferManager::importTransfer(const Transfer &transfer)
{
    if (transfer.state() == Transfer::UndefinedState) {
        return;
    }

    const bool update = m_transfers[transfer.alignment()].contains(transfer.reservationId());
    m_transfers[transfer.alignment()].insert(transfer.reservationId(), transfer);
    writeToFile(transfer);

    update ? emit transferChanged(transfer) : emit transferAdded(transfer);
}

KPublicTransport::JourneyRequest TransferManager::journeyRequestForTransfer(const Transfer &transfer) const
{
    using namespace KPublicTransport;
    JourneyRequest req;
    req.setFrom(transfer.from());
    req.setTo(transfer.to());
    req.setDateTime(transfer.journeyTime());
    req.setDateTimeMode(transfer.alignment() == Transfer::Before ? JourneyRequest::Arrival : JourneyRequest::Departure);
    req.setDownloadAssets(true);
    return req;
}

static KPublicTransport::Journey pickJourney(const Transfer &t, const std::vector<KPublicTransport::Journey> &journeys)
{
    if (journeys.empty()) {
        return {};
    }
    Q_UNUSED(t)
    return journeys[0]; // TODO
}

void TransferManager::autoFillTransfer(Transfer &t)
{
    if (!m_autoFillTransfers || t.state() != Transfer::Pending || !t.hasLocations()) {
        return;
    }

    t.setState(Transfer::Searching);

    auto reply = m_ptrMgr->queryJourney(journeyRequestForTransfer(t));
    const auto batchId = t.reservationId();
    const auto alignment = t.alignment();
    connect(reply, &KPublicTransport::JourneyReply::finished, this, [this, reply, batchId, alignment]() {
        reply->deleteLater();
        auto t = transfer(batchId, alignment);
        if (t.state() != Transfer::Searching) { // user override happened meanwhile
            qDebug() << "ignoring journey reply, transfer state changed";
            return;
        }

        if (reply->error() != KPublicTransport::JourneyReply::NoError) {
            qDebug() << reply->errorString();
            t.setState(reply->error() == KPublicTransport::JourneyReply::NotFoundError ? Transfer::Discarded : Transfer::Pending);
        }

        const auto journeys = std::move(reply->takeResult());
        if (journeys.empty() && t.state() == Transfer::Searching) {
            qDebug() << "no journeys found for transfer, discarding";
            t.setState(Transfer::Discarded);
        }

        const auto journey = pickJourney(t, journeys);
        if (journey.scheduledArrivalTime().isValid()) {
            t.setJourney(journey);
            t.setState(Transfer::Selected);
        } else if (t.state() == Transfer::Searching) {
            t.setState(Transfer::Pending);
        }
        addOrUpdateTransfer(t);
    });
}

QDateTime TransferManager::currentDateTime() const
{
    if (Q_UNLIKELY(m_nowOverride.isValid())) {
        return m_nowOverride;
    }
    return QDateTime::currentDateTime();
}

void TransferManager::overrideCurrentDateTime(const QDateTime &dt)
{
    m_nowOverride = dt;
}

void TransferManager::clear()
{
    QDir d(transferBasePath());
    qCInfo(Log) << "deleting" << transferBasePath();
    d.removeRecursively();

    QSettings settings;
    settings.beginGroup(QStringLiteral("TransferManager"));
    settings.remove(QStringLiteral("FullScan"));
}
