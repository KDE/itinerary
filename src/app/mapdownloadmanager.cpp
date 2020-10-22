/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapdownloadmanager.h"
#include "reservationmanager.h"

#include <KOSMIndoorMap/MapLoader>

#include <KItinerary/LocationUtil>
#include <KItinerary/Place>
#include <KItinerary/SortUtil>

#include <QDateTime>
#include <QDebug>
#include <QTimer>

#include <cmath>

using namespace KItinerary;

MapDownloadManager::MapDownloadManager(QObject* parent)
    : QObject(parent)
{
}

MapDownloadManager::~MapDownloadManager() = default;

void MapDownloadManager::setReservationManager(ReservationManager *resMgr)
{
    m_resMgr = resMgr;

    connect(m_resMgr, &ReservationManager::batchAdded, this, &MapDownloadManager::addAutomaticRequestForBatch);
    connect(m_resMgr, &ReservationManager::batchChanged, this, &MapDownloadManager::addAutomaticRequestForBatch);
    connect(m_resMgr, &ReservationManager::batchContentChanged, this, &MapDownloadManager::addAutomaticRequestForBatch);
}

void MapDownloadManager::setAutomaticDownloadEnabled(bool enable)
{
    m_autoDownloadEnabled = enable;
}

static bool isRelevantTime(const QDateTime &dt)
{
//      const auto now = QDateTime::currentDateTime();
    const auto now = QDateTime({2020,1,30}, {0,0});
    return dt > now && dt < now.addDays(14);
}

void MapDownloadManager::download()
{
    for (const auto &batchId : m_resMgr->batches()) {
        addRequestForBatch(batchId);
    }

    qDebug() << m_pendingRequests.size() << "pending download requests";
    downloadNext();
}

void MapDownloadManager::addAutomaticRequestForBatch(const QString& batchId)
{
    addRequestForBatch(batchId);
    if (m_autoDownloadEnabled) { //  TODO check for the network state, we only want to download this over Wifi automatically
        QTimer::singleShot(std::chrono::seconds(5), this, &MapDownloadManager::downloadNext);
    }
}

void MapDownloadManager::addRequestForBatch(const QString &batchId)
{
    const auto res = m_resMgr->reservation(batchId);
    if (!LocationUtil::isLocationChange(res)) {
        return;
    }
    const auto arrTime = SortUtil::endDateTime(res);
    if (isRelevantTime(arrTime)) {
        const auto arr = LocationUtil::arrivalLocation(res);
        const auto arrGeo = LocationUtil::geo(arr);
        qDebug() << LocationUtil::name(arr) << arrGeo.latitude() << arrGeo.longitude();
        addRequest(arrGeo.latitude(), arrGeo.longitude(), arrTime);
    }

    const auto depTime = SortUtil::startDateTime(res);
    if (isRelevantTime(depTime)) {
        const auto dep = LocationUtil::departureLocation(res);
        const auto depGeo = LocationUtil::geo(dep);
        qDebug() << LocationUtil::name(dep) << depGeo.latitude() << depGeo.longitude();
        addRequest(depGeo.latitude(), depGeo.longitude(), depTime);
    }
}

void MapDownloadManager::addRequest(double lat, double lon, const QDateTime &cacheUntil)
{
    if (std::isnan(lat) || std::isnan(lon)) {
        return;
    }

    for (auto &req : m_pendingRequests) {
        if (LocationUtil::distance(req.lat, req.lon, lat, lon) < 10.0) {
            req.cacheUntil = std::max(req.cacheUntil, cacheUntil);
            return;
        }
    }

    m_pendingRequests.push_back({ lat, lon, cacheUntil });
}

void MapDownloadManager::downloadNext()
{
    if (m_loader || m_pendingRequests.empty()) {
        return;
    }

    const auto req = std::move(m_pendingRequests.back());
    m_pendingRequests.pop_back();

    m_loader = new KOSMIndoorMap::MapLoader(this);
    connect(m_loader, &KOSMIndoorMap::MapLoader::done, this, &MapDownloadManager::downloadFinished);
    m_loader->loadForCoordinate(req.lat, req.lon, req.cacheUntil);
}

void MapDownloadManager::downloadFinished()
{
    m_loader->deleteLater();
    m_loader = nullptr;

    if (m_pendingRequests.empty()) {
        emit finished();
    } else {
        downloadNext();
    }
}
