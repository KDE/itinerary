/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MAPDOWNLOADMANAGER_H
#define MAPDOWNLOADMANAGER_H

#include <QDateTime>
#include <QObject>

#include <vector>

class ReservationManager;

namespace SolidExtras
{
class NetworkStatus;
}

namespace KOSMIndoorMap
{
class MapLoader;
}

/** Downloads indoor map tiles for upcoming trips. */
class MapDownloadManager : public QObject
{
    Q_OBJECT
public:
    explicit MapDownloadManager(QObject *parent = nullptr);
    ~MapDownloadManager() override;

    void setReservationManager(ReservationManager *resMgr);
    // TODO always process complete trip groups?
    // TODO should we also handle transfer locations?

    /** Trigger download of missing map data immediately, regardless of the current network state. */
    Q_INVOKABLE void download();

    void setAutomaticDownloadEnabled(bool enable);

    // TODO progress reporting, cancel?

Q_SIGNALS:
    /** All pending downloads have been completed. */
    void finished();

private:
    bool canAutoDownload() const;
    void addAutomaticRequestForBatch(const QString &batchId);
    void addRequestForBatch(const QString &batchId);
    void addRequest(double lat, double lon, const QDateTime &cacheUntil);
    void downloadNext();
    void downloadFinished();
    void networkStatusChanged();

    ReservationManager *m_resMgr = nullptr;
    bool m_autoDownloadEnabled = false;

    struct Request {
        double lat;
        double lon;
        QDateTime cacheUntil;
    };
    std::vector<Request> m_pendingRequests;
    std::vector<Request> m_cachedRequests;
    Request m_currentRequest;

    KOSMIndoorMap::MapLoader *m_loader = nullptr;
    SolidExtras::NetworkStatus *m_netStatus = nullptr;
};

#endif // MAPDOWNLOADMANAGER_H
