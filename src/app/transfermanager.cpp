/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "transfermanager.h"
#include "logging.h"
#include "publictransport.h"
#include "reservationmanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>

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

TransferManager* TransferManager::s_instance = nullptr;

TransferManager::TransferManager(QObject *parent)
    : QObject(parent)
{
    s_instance = this;

    QSettings settings;
    settings.beginGroup(QStringLiteral("HomeLocation"));
    m_homeLat = settings.value(QStringLiteral("Latitude"), NAN).toFloat();
    m_homeLon = settings.value(QStringLiteral("Longitude"), NAN).toFloat();

    connect(this, &TransferManager::homeLocationChanged, this, [this]() { rescan(true); });
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

void TransferManager::discardTransfer(Transfer transfer)
{
    transfer.setState(Transfer::Discarded);
    transfer.setJourney({});
    m_transfers[transfer.alignment()].insert(transfer.reservationId(), transfer);
    writeToFile(transfer);
    emit transferRemoved(transfer.reservationId(), transfer.alignment());
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

    return alignment == Transfer::Before ? checkTransferBefore(resId, res, t) : checkTransferAfter(resId, res, t);
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

    if (alignment == Transfer::Before ? checkTransferBefore(resId, res, t) : checkTransferAfter(resId, res, t)) {
        addOrUpdateTransfer(t);
        return t;
    } else {
        return {};
    }
}

float TransferManager::homeLatitude() const
{
    return m_homeLat;
}

void TransferManager::setHomeLatitude(float lat)
{
    if (m_homeLat == lat) {
        return;
    }
    m_homeLat = lat;
    QSettings settings;
    settings.beginGroup(QStringLiteral("HomeLocation"));
    settings.setValue(QStringLiteral("Latitude"), m_homeLat);
    emit homeLocationChanged();
}

float TransferManager::homeLongitude() const
{
    return m_homeLon;
}

void TransferManager::setHomeLongitude(float lon)
{
    if (m_homeLon == lon) {
        return;
    }
    m_homeLon = lon;
    QSettings settings;
    settings.beginGroup(QStringLiteral("HomeLocation"));
    settings.setValue(QStringLiteral("Longitude"), m_homeLon);
    emit homeLocationChanged();
}

bool TransferManager::hasHomeLocation() const
{
    return !std::isnan(m_homeLat) && !std::isnan(m_homeLon);
}

KPublicTransport::Location TransferManager::homeLocation() const
{
    if (!hasHomeLocation()) {
        return {};
    }
    KPublicTransport::Location l;
    l.setName(i18n("Home"));
    l.setCoordinate(m_homeLat, m_homeLon);
    return l;
}

void TransferManager::rescan(bool force)
{
    if (!m_resMgr || !m_tgMgr) {
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

    if (alignment == Transfer::Before ? checkTransferBefore(resId, res, t) : checkTransferAfter(resId, res, t)) {
        addOrUpdateTransfer(t);
    } else {
        removeTransfer(t);
    }
}

bool TransferManager::checkTransferBefore(const QString &resId, const QVariant &res, Transfer &transfer) const
{
    transfer.setAnchorTime(SortUtil::startDateTime(res));
    const auto isLocationChange = LocationUtil::isLocationChange(res);
    if (isLocationChange) {
        transfer.setTo(PublicTransport::locationFromPlace(LocationUtil::departureLocation(res), res));
    } else {
        transfer.setTo(PublicTransport::locationFromPlace(LocationUtil::location(res), res));
    }

    // TODO pre-transfers should happen in the following cases:
    // - res is a location change and we are currently at home (== first element in a trip group)
    // - res is a location change and we are not at the departure location yet
    // - res is an event and we are not at its location already

    if (isLocationChange && isFirstInTripGroup(resId)) {
        transfer.setFrom(homeLocation());
        return true;
    }

    if (isLocationChange) {
        const auto prevResId = m_resMgr->previousBatch(resId); // TODO this fails for multiple nested range elements!
        if (prevResId.isEmpty()) {
            return false;
        }
        const auto prevRes = m_resMgr->reservation(prevResId);
        const auto curLoc = LocationUtil::departureLocation(res);
        QVariant prevLoc;
        if (LocationUtil::isLocationChange(prevRes)) {
            prevLoc = LocationUtil::arrivalLocation(prevRes);
        } else {
            prevLoc = LocationUtil::location(prevRes);
        }
        if (!curLoc.isNull() && !prevLoc.isNull() && !LocationUtil::isSameLocation(curLoc, prevLoc, LocationUtil::WalkingDistance)) {
            qDebug() << res << prevRes << LocationUtil::name(curLoc) << LocationUtil::name(prevLoc);
            transfer.setTo(PublicTransport::locationFromPlace(prevLoc, prevRes));
            return true;
        }
    }

    // TODO

    return false;
}

bool TransferManager::checkTransferAfter(const QString &resId, const QVariant &res, Transfer &transfer) const
{
    transfer.setAnchorTime(SortUtil::endDateTime(res));
    const auto isLocationChange = LocationUtil::isLocationChange(res);
    if (isLocationChange) {
        transfer.setFrom(PublicTransport::locationFromPlace(LocationUtil::arrivalLocation(res), res));
    } else {
        transfer.setFrom(PublicTransport::locationFromPlace(LocationUtil::location(res), res));
    }

    // TODO post-transfer should happen in the following cases:
    // - res is a location change and we are the last element in a trip group (ie. going home)
    // - res is a location change and the following element is in a different location, or has a different departure location
    // - res is an event and the following or enclosing element is a lodging element

    if (isLocationChange && isLastInTripGroup(resId)) {
        transfer.setTo(homeLocation());
        return true;
    }

    if (isLocationChange) {
        const auto nextResId = m_resMgr->nextBatch(resId);
        if (nextResId.isEmpty()) {
            return false;
        }
        const auto nextRes = m_resMgr->reservation(nextResId);
        const auto curLoc = LocationUtil::arrivalLocation(res);
        QVariant nextLoc;
        if (LocationUtil::isLocationChange(nextRes)) {
            nextLoc = LocationUtil::departureLocation(nextRes);
        } else {
            nextLoc = LocationUtil::location(nextRes);
        }
        if (!curLoc.isNull() && !nextLoc.isNull() && !LocationUtil::isSameLocation(curLoc, nextLoc, LocationUtil::WalkingDistance)) {
            qDebug() << res << nextRes << LocationUtil::name(curLoc) << LocationUtil::name(nextLoc);
            transfer.setTo(PublicTransport::locationFromPlace(nextLoc, nextRes));
            return true;
        }
    }

    // TODO

    return false;
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

void TransferManager::addOrUpdateTransfer(Transfer &t)
{
    if (t.state() == Transfer::UndefinedState) { // newly added
        if (!t.hasLocations()) { // undefined home location
            return;
        }
        t.setState(Transfer::Pending);
        m_transfers[t.alignment()].insert(t.reservationId(), t);
        writeToFile(t);
        emit transferAdded(t);
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
    const QString fileName = transferBasePath() + resId + (alignment == Transfer::Before ? QLatin1String("-BEFORE.json") : QLatin1String("-AFTER.json"));
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly)) {
        return {};
    }
    return Transfer::fromJson(QJsonDocument::fromJson(f.readAll()).object());
}

void TransferManager::writeToFile(const Transfer &transfer) const
{
    QDir().mkpath(transferBasePath());
    const QString fileName = transferBasePath() + transfer.reservationId() + (transfer.alignment() == Transfer::Before ? QLatin1String("-BEFORE.json") : QLatin1String("-AFTER.json"));
    QFile f(fileName);
    if (!f.open(QFile::WriteOnly)) {
        qCWarning(Log) << "Failed to store transfer data" << f.fileName() << f.errorString();
        return;
    }
    f.write(QJsonDocument(Transfer::toJson(transfer)).toJson());
}

void TransferManager::removeFile(const QString &resId, Transfer::Alignment alignment) const
{
    const QString fileName = transferBasePath() + resId + (alignment == Transfer::Before ? QLatin1String("-BEFORE.json") : QLatin1String("-AFTER.json"));
    QFile::remove(fileName);
}

TransferManager* TransferManager::instance()
{
    return s_instance;
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
