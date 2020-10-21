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

namespace KOSMIndoorMap {
class MapLoader;
}

/** Downloads indoor map tiles for upcoming trips. */
class MapDownloadManager : public QObject
{
    Q_OBJECT
public:
    explicit MapDownloadManager(QObject *parent = nullptr);
    ~MapDownloadManager();

    void setReservationManager(ReservationManager *resMgr);
    // TODO always process complete trip groups?
    // TODO should we also handle transfer locations?

    /** Trigger download of missing map data immediately, regardless of the current network state. */
    Q_INVOKABLE void download();

    // TODO auto download options, considering network state
    // TODO progress reporting, cancel?

Q_SIGNALS:
    /** All pending downloads have been completed. */
    void finished();

private:
    void addRequest(double lat, double lon, const QDateTime &cacheUntil);
    void downloadNext();
    void downloadFinished();

    ReservationManager *m_resMgr = nullptr;

    struct Request {
        double lat;
        double lon;
        QDateTime cacheUntil;
    };
    std::vector<Request> m_pendingRequests;

    KOSMIndoorMap::MapLoader *m_loader = nullptr;
};

#endif // MAPDOWNLOADMANAGER_H
