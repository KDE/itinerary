/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapdownloadmanager.h"
#include "reservationmanager.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/Place>
#include <KItinerary/SortUtil>

#include <QDateTime>
#include <QDebug>

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

    // TODO watch for changes
}

static bool isRelevantTime(const QDateTime &dt)
{
    const auto now = QDateTime::currentDateTime();
    return dt > now && dt < now.addDays(14);
}

void MapDownloadManager::download()
{
    for (const auto &batchId : m_resMgr->batches()) {
        const auto res = m_resMgr->reservation(batchId);
        if (!LocationUtil::isLocationChange(res)) {
            continue;
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

    qDebug() << m_pendingRequests.size() << "pending download requests";
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
