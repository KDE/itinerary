/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "transfermanager.h"

#include "constants.h"
#include "jsonio.h"
#include "logging.h"
#include "favoritelocationmodel.h"
#include "livedatamanager.h"
#include "publictransport.h"
#include "reservationhelper.h"
#include "reservationmanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"

#include <KItinerary/BoatTrip>
#include <KItinerary/BusTrip>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/LocationUtil>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/Journey>
#include <KPublicTransport/JourneyReply>
#include <KPublicTransport/Manager>

#include <KLocalizedString>

#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QSettings>
#include <QStandardPaths>

using namespace KItinerary;

// bump this to trigger a full rescan for transfers
enum { CurrentFullScanVersion = 1 };

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

void TransferManager::setLiveDataManager(LiveDataManager *liveDataMgr)
{
    m_liveDataMgr = liveDataMgr;
    connect(m_liveDataMgr, &LiveDataManager::arrivalUpdated,this, [this](const QString &resId) {
        // update anchor time if we have a transfer for this
        auto t = transfer(resId, Transfer::After);
        if (t.state() == Transfer::Discarded || t.state() == Transfer::UndefinedState) {
            return;
        }

        t.setAnchorTime(anchorTimeAfter(resId, m_resMgr->reservation(resId)));
        addOrUpdateTransfer(t);

        // TODO if there's existing transfer, check if we miss this now
        // if so: warn and search for a new one if auto transfers are enabled
    });
    connect(m_liveDataMgr, &LiveDataManager::journeyUpdated, this, qOverload<const QString&>(&TransferManager::checkReservation), Qt::QueuedConnection);
    rescan();
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
    Q_EMIT transferChanged(transfer);
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
    Q_EMIT transferChanged(transfer);

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

    const bool canAdd = (alignment == Transfer::Before ? checkTransferBefore(resId, res, t) : checkTransferAfter(resId, res, t)) != ShouldRemove;
    return canAdd && t.hasLocations() && t.anchorTime().isValid();
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
    if (!m_resMgr || !m_tgMgr || !m_favLocModel || !m_autoAddTransfers || !m_liveDataMgr) {
        return;
    }

    QSettings settings;
    settings.beginGroup(QStringLiteral("TransferManager"));
    const auto previousFullScanVersion = settings.value(QLatin1StringView("FullScan"), 0).toInt();
    if (!force && previousFullScanVersion >= CurrentFullScanVersion) {
        return;
    }

    qCInfo(Log) << "Performing a full transfer search..." << previousFullScanVersion;
    for (const auto &batchId : m_resMgr->batches()) {
        checkReservation(batchId);
    }
    settings.setValue(QStringLiteral("FullScan"), CurrentFullScanVersion);
}

void TransferManager::checkReservation(const QString &resId)
{
    if (!m_autoAddTransfers) {
        return;
    }

    const auto res = m_resMgr->reservation(resId);

    checkReservation(resId, res, Transfer::After);
    // also check before the following reservation
    auto nextResId = resId;
    while (true) {
        nextResId = m_resMgr->nextBatch(nextResId);
        if (nextResId.isEmpty()) {
            break;
        }
        const auto nextRes = m_resMgr->reservation(nextResId);
        if (!ReservationHelper::isCancelled(nextRes)) {
            checkReservation(nextResId, nextRes, Transfer::Before);
            break;
        }
    }

    checkReservation(resId, res, Transfer::Before);
    // also check after the previous reservation
    auto prevResId = resId;
    while (true) {
        prevResId = m_resMgr->previousBatch(prevResId);
        if (prevResId.isEmpty()) {
            break;
        }
        const auto prevRes = m_resMgr->reservation(prevResId);
        if (!ReservationHelper::isCancelled(prevRes)) {
            checkReservation(prevResId, prevRes, Transfer::After);
            break;
        }
    }
}

void TransferManager::checkReservation(const QString &resId, const QVariant &res, Transfer::Alignment alignment)
{
    const auto anchorTime = alignment == Transfer::Before ? anchorTimeBefore(resId, res) : anchorTimeAfter(resId, res);
    if (!anchorTime.isValid() || anchorTime < currentDateTime()) {
        return;
    }

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

/** Checks whether @p loc1 and @p loc2 are far enough apart to need a tranfer,
 *  with sufficient certainty.
 */
static bool isLikelyNotSameLocation(const QVariant &loc1, const QVariant &loc2)
{
    // if we are sure they are the same location we are done here
    if (LocationUtil::isSameLocation(loc1, loc2, LocationUtil::WalkingDistance)) {
        return false;
    }

    // if both have a geo coordinate we are also sure about both being different from the above check
    if (LocationUtil::geo(loc1).isValid() && LocationUtil::geo(loc2).isValid()) {
        return true;
    }

    // if we have to rely on the name, only do that if we are really sure they are different
    return !LocationUtil::isSameLocation(loc1, loc2, LocationUtil::CityLevel);
}

/** Check whether @p loc1 and @p loc2 have a realistic distance for a transfer,
 *  assuming we know their geo coodinates.
 *  This helps filtering out non-sense transfers if we end up with entries in the wrong order.
 */
static bool isPlausibleDistance(const QVariant &loc1, const QVariant &loc2)
{
    const auto geo1 = LocationUtil::geo(loc1);
    const auto geo2 = LocationUtil::geo(loc2);
    if (!geo1.isValid() || !geo2.isValid()) {
        return true;
    }
    return LocationUtil::distance(geo1, geo2) < 100'000;
}

TransferManager::CheckTransferResult TransferManager::checkTransferBefore(const QString &resId, const QVariant &res, Transfer &transfer) const
{
    if (ReservationHelper::isCancelled(res) || !SortUtil::hasStartTime(res)) {
        return ShouldRemove;
    }

    transfer.setAnchorTime(anchorTimeBefore(resId, res));
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
    // - res is a location change, and the previous element is also a location change but not a connection
    //   (ie. transfer from favorite location at the destination of a roundtrip trip group)


    const auto notInGroup = isNotInTripGroup(resId);
    if ((isLocationChange && isFirstInTripGroup(resId)) || notInGroup) {
        const auto f = pickFavorite(toLoc, resId, Transfer::Before);
        transfer.setFrom(locationFromFavorite(f));
        transfer.setFromName(f.name());
        transfer.setFloatingLocationType(Transfer::FavoriteLocation);
        return notInGroup ? CanAddManually : ShouldAutoAdd;
    }

    // find the first preceeding non-cancelled reservation
    QString prevResId = resId;
    QVariant prevRes;
    while (true) {
        prevResId = m_resMgr->previousBatch(prevResId); // TODO this fails for multiple nested range elements!
        if (prevResId.isEmpty()) {
            return ShouldRemove;
        }
        prevRes = m_resMgr->reservation(prevResId);
        if (!ReservationHelper::isCancelled(prevRes)) {
            break;
        }
    }

    // check if there is a transfer after prevRes already
    const auto prevTransfer = this->transfer(prevResId, Transfer::After);
    if (prevTransfer.state() != Transfer::UndefinedState && prevTransfer.state() != Transfer::Discarded) {
        if (prevTransfer.floatingLocationType() == Transfer::FavoriteLocation) {
            transfer.setFrom(prevTransfer.to());
            transfer.setFromName(prevTransfer.toName());
            transfer.setFloatingLocationType(Transfer::FavoriteLocation);
            return CanAddManually;
        }
        return ShouldRemove;
    }

    QVariant prevLoc;
    if (LocationUtil::isLocationChange(prevRes)) {
        prevLoc = LocationUtil::arrivalLocation(prevRes);
    } else {
        prevLoc = LocationUtil::location(prevRes);
    }
    if (!toLoc.isNull() && !prevLoc.isNull() && isLikelyNotSameLocation(toLoc, prevLoc) && isPlausibleDistance(toLoc, prevLoc)) {
        qDebug() << res << prevRes << LocationUtil::name(toLoc) << LocationUtil::name(prevLoc) << transfer.anchorTime();
        transfer.setFrom(PublicTransport::locationFromPlace(prevLoc, prevRes));
        transfer.setFromName(LocationUtil::name(prevLoc));
        transfer.setFloatingLocationType(Transfer::Reservation);
        return isLocationChange ? ShouldAutoAdd : CanAddManually;
    }

    // transfer to favorite at destination of a roundtrip trip group
    if (LocationUtil::isLocationChange(res) && LocationUtil::isLocationChange(prevRes) && LocationUtil::isSameLocation(toLoc, prevLoc)) {
        const auto arrivalTime = SortUtil::endDateTime(prevRes);
        const auto departureTime = SortUtil::startDateTime(res);
        transfer.setFloatingLocationType(Transfer::FavoriteLocation);
        const auto f = pickFavorite(toLoc, resId, Transfer::Before);
        transfer.setFrom(locationFromFavorite(f));
        transfer.setFromName(f.name());
        return std::chrono::seconds(arrivalTime.secsTo(departureTime)) < Constants::MaximumLayoverTime ? ShouldRemove : CanAddManually;
    }

    return ShouldRemove;
}

TransferManager::CheckTransferResult TransferManager::checkTransferAfter(const QString &resId, const QVariant &res, Transfer &transfer) const
{
    if (ReservationHelper::isCancelled(res) || !SortUtil::hasEndTime(res)) {
        return ShouldRemove;
    }

    transfer.setAnchorTime(anchorTimeAfter(resId, res));
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
    // - res is a location change, and the subsequent element is also a location change but not a connection
    //   (ie. transfer to favorite location at the destination of a roundtrip trip group)

    const auto notInGroup = isNotInTripGroup(resId);
    if ((isLocationChange && isLastInTripGroup(resId)) || notInGroup) {
        const auto f = pickFavorite(fromLoc, resId, Transfer::After);
        transfer.setTo(locationFromFavorite(f));
        transfer.setToName(f.name());
        transfer.setFloatingLocationType(Transfer::FavoriteLocation);
        return notInGroup ? CanAddManually : ShouldAutoAdd;
    }

    // find next non-cancelled reservation
    QString nextResId = resId;
    QVariant nextRes;
    while (true) {
        nextResId = m_resMgr->nextBatch(nextResId);
        if (nextResId.isEmpty()) {
            return ShouldRemove;
        }
        nextRes = m_resMgr->reservation(nextResId);
        if (!ReservationHelper::isCancelled(nextRes)) {
            break;
        }
    }

    // check if there is a transfer before nextRes already
    const auto nextTransfer = this->transfer(nextResId, Transfer::Before);
    if (nextTransfer.state() != Transfer::UndefinedState && nextTransfer.state() != Transfer::Discarded) {
        if (nextTransfer.floatingLocationType() == Transfer::FavoriteLocation) {
            transfer.setTo(nextTransfer.from());
            transfer.setToName(nextTransfer.fromName());
            transfer.setFloatingLocationType(Transfer::FavoriteLocation);
            return CanAddManually;
        }
        return ShouldRemove;
    }

    QVariant nextLoc;
    if (LocationUtil::isLocationChange(nextRes)) {
        nextLoc = LocationUtil::departureLocation(nextRes);
    } else {
        nextLoc = LocationUtil::location(nextRes);
    }
    if (!fromLoc.isNull() && !nextLoc.isNull() && isLikelyNotSameLocation(fromLoc, nextLoc) && isPlausibleDistance(fromLoc, nextLoc)) {
        qDebug() << res << nextRes << LocationUtil::name(fromLoc) << LocationUtil::name(nextLoc) << transfer.anchorTime();
        transfer.setTo(PublicTransport::locationFromPlace(nextLoc, nextRes));
        transfer.setToName(LocationUtil::name(nextLoc));
        transfer.setFloatingLocationType(Transfer::Reservation);
        return isLocationChange ? ShouldAutoAdd : CanAddManually;
    }

    // transfer to favorite at destination of a roundtrip trip group
    if (LocationUtil::isLocationChange(res) && LocationUtil::isLocationChange(nextRes) && LocationUtil::isSameLocation(fromLoc, nextLoc)) {
        const auto arrivalTime = SortUtil::endDateTime(res);
        const auto departureTime = SortUtil::startDateTime(nextRes);
        transfer.setFloatingLocationType(Transfer::FavoriteLocation);
        const auto f = pickFavorite(fromLoc, resId, Transfer::After);
        transfer.setTo(locationFromFavorite(f));
        transfer.setToName(f.name());
        return std::chrono::seconds(arrivalTime.secsTo(departureTime)) < Constants::MaximumLayoverTime ? ShouldRemove : CanAddManually;
    }

    return ShouldRemove;
}

void TransferManager::reservationRemoved(const QString &resId)
{
    m_transfers[Transfer::Before].remove(resId);
    m_transfers[Transfer::After].remove(resId);
    removeFile(resId, Transfer::Before);
    removeFile(resId, Transfer::After);
    // TODO updates to adjacent transfers?
    Q_EMIT transferRemoved(resId, Transfer::Before);
    Q_EMIT transferRemoved(resId, Transfer::After);
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

// default transfer anchor deltas (in minutes)
enum { FlightDelta, TrainDelta, BusDelta, BoatDelta, RestaurantDelta, FallbackDelta };
static constexpr const int default_deltas[][2] = {
    { 90, 30 }, // Flight
    { 20, 10 }, // Train
    { 15, 10 }, // Bus
    { 60, 30 }, // Boat/Ferry
    {  5,  5 }, // Restaurant
    { 30, 15 }, // anything else
};

void TransferManager::determineAnchorDeltaDefault(Transfer &transfer, const QVariant &res) const
{
    if (transfer.state() != Transfer::UndefinedState) {
        return;
    }

    int delta;
    if (JsonLd::isA<FlightReservation>(res)) {
        delta = default_deltas[FlightDelta][transfer.alignment()];
    } else if (JsonLd::isA<TrainReservation>(res)) {
        delta = default_deltas[TrainDelta][transfer.alignment()];
    } else if (JsonLd::isA<BusReservation>(res)) {
        delta = default_deltas[BusDelta][transfer.alignment()];
    } else if (JsonLd::isA<BoatReservation>(res)) {
        delta = default_deltas[BoatDelta][transfer.alignment()];
    } else if (JsonLd::isA<FoodEstablishmentReservation>(res)) {
        delta = default_deltas[RestaurantDelta][transfer.alignment()];
    } else {
        delta = default_deltas[FallbackDelta][transfer.alignment()];
    }
    transfer.setAnchorTimeDelta(delta * 60);
}

QDateTime TransferManager::anchorTimeBefore(const QString &resId, const QVariant &res) const
{
    if (JsonLd::isA<TrainReservation>(res)) {
        const auto departure = m_liveDataMgr->departure(resId);
        if (departure.hasExpectedDepartureTime()) {
            return departure.expectedDepartureTime();
        }
    }
        if (JsonLd::isA<FlightReservation>(res)) {
        const auto flight = res.value<FlightReservation>().reservationFor().value<Flight>();
        if (flight.boardingTime().isValid()) {
            return flight.boardingTime();
        }
    }
    if (LocationUtil::isLocationChange(res)) {
        return SortUtil::startDateTime(res);
    }

    if (JsonLd::isA<EventReservation>(res)) {
        const auto event = res.value<EventReservation>().reservationFor().value<Event>();
        if (event.doorTime().isValid()) {
            return event.doorTime();
        }
        return event.startDate();
    }
    if (JsonLd::isA<FoodEstablishmentReservation>(res)) {
        return res.value<FoodEstablishmentReservation>().startTime();
    }

    return {};
}

QDateTime TransferManager::anchorTimeAfter(const QString &resId, const QVariant &res) const
{
    if (JsonLd::isA<TrainReservation>(res)) {
        const auto arrival = m_liveDataMgr->arrival(resId);
        if (arrival.hasExpectedArrivalTime()) {
            return arrival.expectedArrivalTime();
        }
    }
    if (LocationUtil::isLocationChange(res)) {
        return SortUtil::endDateTime(res);
    }

    if (JsonLd::isA<EventReservation>(res)) {
        return res.value<EventReservation>().reservationFor().value<Event>().endDate();
    }
    if (JsonLd::isA<FoodEstablishmentReservation>(res)) {
        return res.value<FoodEstablishmentReservation>().endTime();
    }

    return {};
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
    const auto &favLocs = m_favLocModel->favoriteLocations();
    if (favLocs.empty()) {
        return {};
    }

    // TODO selection strategy:
    // (1) pick the same favorite as was used before/after resId
    // (2) pick the favorite closest to anchoredLoc - this can work very well if the favorites aren't close to each other
    // (3) pick the first one

    Q_UNUSED(resId)
    Q_UNUSED(alignment)

    // pick the first location within a 50km distance
    const auto anchordCoord = LocationUtil::geo(anchoredLoc);
    if (!anchordCoord.isValid()) {
        return {};
    }
    const auto it = std::find_if(favLocs.begin(), favLocs.end(), [&anchordCoord](const auto &fav) {
        const auto d = LocationUtil::distance(anchordCoord.latitude(), anchordCoord.longitude(), fav.latitude(), fav.longitude());
        return d < Constants::MaximumFavoriteLocationTransferDistance;
    });
    if (it != favLocs.end()) {
        return (*it);
    }
    return {};
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
        Q_EMIT transferAdded(t);
    } else if (t.state() == Transfer::Discarded) {
        m_transfers[t.alignment()].insert(t.reservationId(), t);
        writeToFile(t);
        Q_EMIT transferRemoved(t.reservationId(), t.alignment());
    } else { // update existing data
        if (t == transfer(t.reservationId(), t.alignment())) {
            return;
        }
        m_transfers[t.alignment()].insert(t.reservationId(), t);
        writeToFile(t);
        Q_EMIT transferChanged(t);
    }
}

void TransferManager::removeTransfer(const Transfer &t)
{
    if (t.state() == Transfer::UndefinedState) { // this was never added
        return;
    }
    m_transfers[t.alignment()].remove(t.reservationId());
    removeFile(t.reservationId(), t.alignment());
    Q_EMIT transferRemoved(t.reservationId(), t.alignment());
}

static QString transferBasePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1StringView("/transfers/");
}

Transfer TransferManager::readFromFile(const QString& resId, Transfer::Alignment alignment) const
{
    const QString fileName = transferBasePath() + Transfer::identifier(resId, alignment) + QLatin1StringView(".json");
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly)) {
        return {};
    }
    return Transfer::fromJson(JsonIO::read(f.readAll()).toObject());
}

void TransferManager::writeToFile(const Transfer &transfer) const
{
    QDir().mkpath(transferBasePath());
    const QString fileName = transferBasePath() + transfer.identifier() + QLatin1StringView(".json");
    QFile f(fileName);
    if (!f.open(QFile::WriteOnly)) {
        qCWarning(Log) << "Failed to store transfer data" << f.fileName() << f.errorString();
        return;
    }
    f.write(JsonIO::write(Transfer::toJson(transfer)));
}

void TransferManager::removeFile(const QString &resId, Transfer::Alignment alignment) const
{
    const QString fileName = transferBasePath() + Transfer::identifier(resId, alignment) + QLatin1StringView(".json");
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

    update ? Q_EMIT transferChanged(transfer) : Q_EMIT transferAdded(transfer);
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
    req.setIncludeIntermediateStops(true);
    req.setIncludePaths(true);
    req.setMaximumResults(6);
    return req;
}

static KPublicTransport::Journey pickJourney(const Transfer &t, const std::vector<KPublicTransport::Journey> &journeys)
{
    if (journeys.empty()) {
        return {};
    }
    return t.alignment() == Transfer::Before ? journeys.back() : journeys.front();
}

void TransferManager::autoFillTransfer(Transfer &t)
{
    if (!m_autoFillTransfers || t.state() != Transfer::Pending || !t.hasLocations()) {
        return;
    }

    t.setState(Transfer::Searching);

    auto reply = m_liveDataMgr->publicTransportManager()->queryJourney(journeyRequestForTransfer(t));
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

#include "moc_transfermanager.cpp"
