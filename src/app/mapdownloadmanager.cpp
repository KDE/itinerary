/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "logging.h"
#include "mapdownloadmanager.h"
#include "reservationmanager.h"

#include <KOSMIndoorMap/MapLoader>

#include <KItinerary/LocationUtil>
#include <KItinerary/Place>
#include <KItinerary/SortUtil>

#include <QNetworkInformation>
#include <QTimer>

#include <cmath>

using namespace KItinerary;

MapDownloadManager::MapDownloadManager(QObject *parent)
    : QObject(parent)
{
    QNetworkInformation::loadDefaultBackend();
    auto netInfo = QNetworkInformation::instance();
    connect(netInfo, &QNetworkInformation::reachabilityChanged, this, &MapDownloadManager::networkStatusChanged);
    connect(netInfo, &QNetworkInformation::isMeteredChanged, this, &MapDownloadManager::networkStatusChanged);
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
    if (canAutoDownload()) {
        download();
    }
}

static bool isRelevantTime(const QDateTime &dt)
{
    const auto now = QDateTime::currentDateTime();
    return dt > now && dt < now.addDays(14);
}

void MapDownloadManager::download()
{
    for (const auto &batchId : m_resMgr->batches()) {
        addRequestForBatch(batchId);
    }

    const auto netInfo = QNetworkInformation::instance();
    qCDebug(Log) << m_pendingRequests.size() << "pending download requests" << netInfo->reachability() << netInfo->isMetered();
    downloadNext();
}

void MapDownloadManager::download(const QStringList &batchIds)
{
    for (const auto &batchId : batchIds) {
        addRequestForBatch(batchId, true);
    }

    const auto netInfo = QNetworkInformation::instance();
    qCDebug(Log) << m_pendingRequests.size() << "pending download requests" << netInfo->reachability() << netInfo->isMetered();
    downloadNext();
}

bool MapDownloadManager::canAutoDownload() const
{
    const auto netInfo = QNetworkInformation::instance();
    return m_autoDownloadEnabled && (netInfo->reachability() == QNetworkInformation::Reachability::Online || netInfo->reachability() == QNetworkInformation::Reachability::Unknown) && !netInfo->isMetered();
}

void MapDownloadManager::addAutomaticRequestForBatch(const QString &batchId)
{
    addRequestForBatch(batchId);
    if (canAutoDownload()) {
        QTimer::singleShot(std::chrono::seconds(5), this, &MapDownloadManager::downloadNext);
    }
}

void MapDownloadManager::addRequestForBatch(const QString &batchId, bool force)
{
    const auto res = m_resMgr->reservation(batchId);
    if (!LocationUtil::isLocationChange(res)) {
        return;
    }
    const auto arrTime = SortUtil::endDateTime(res);
    if (force || isRelevantTime(arrTime)) {
        const auto arr = LocationUtil::arrivalLocation(res);
        const auto arrGeo = LocationUtil::geo(arr);
        qDebug() << LocationUtil::name(arr) << arrGeo.latitude() << arrGeo.longitude();
        addRequest(arrGeo.latitude(), arrGeo.longitude(), arrTime);
    }

    const auto depTime = SortUtil::startDateTime(res);
    if (force || isRelevantTime(depTime)) {
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

    // check if we already have this cached
    for (auto it = m_cachedRequests.begin(); it != m_cachedRequests.end(); ++it) {
        if (LocationUtil::distance(lat, lon, (*it).lat, (*it).lon) > 10.0) {
            continue;
        }
        if ((*it).cacheUntil >= cacheUntil) {
            return;
        }
        m_cachedRequests.erase(it);
        break;
    }

    // check if there is a pending request that would cover this
    for (auto &req : m_pendingRequests) {
        if (LocationUtil::distance(req.lat, req.lon, lat, lon) < 10.0) {
            req.cacheUntil = std::max(req.cacheUntil, cacheUntil);
            return;
        }
    }

    m_pendingRequests.push_back({lat, lon, cacheUntil});
}

void MapDownloadManager::downloadNext()
{
    if (m_loader || m_pendingRequests.empty()) {
        return;
    }

    m_currentRequest = std::move(m_pendingRequests.back());
    m_pendingRequests.pop_back();

    m_loader = new KOSMIndoorMap::MapLoader(this);
    connect(m_loader, &KOSMIndoorMap::MapLoader::done, this, &MapDownloadManager::downloadFinished);
    m_loader->loadForCoordinate(m_currentRequest.lat, m_currentRequest.lon, m_currentRequest.cacheUntil);
}

void MapDownloadManager::downloadFinished()
{
    m_loader->deleteLater();
    m_loader = nullptr;
    m_cachedRequests.push_back(std::move(m_currentRequest));

    if (m_pendingRequests.empty()) {
        Q_EMIT finished();
    } else {
        downloadNext();
    }
}

void MapDownloadManager::networkStatusChanged()
{
    if (canAutoDownload()) {
        downloadNext();
    }
}

#include "moc_mapdownloadmanager.cpp"
