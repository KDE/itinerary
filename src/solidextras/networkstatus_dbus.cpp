/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-solid-extras.h"
#include "networkstatus.h"
#include "portalnetworkmonitor.h"

#ifdef HAVE_NM
#include <NetworkManagerQt/Manager>
#endif

#include <QCoreApplication>
#include <QDebug>

namespace SolidExtras {
class PortalNetworkMonitor : public QObject
{
    Q_OBJECT
public:
    static bool hasPortal();
    static NetworkStatus::State connectivity() { return instance()->m_connected; }
    static NetworkStatus::State metered() { return instance()->m_metered; }
    static PortalNetworkMonitor* instance();

Q_SIGNALS:
    void connectivityChanged();
    void meteredChanged();

private:
    PortalNetworkMonitor(QObject *parent = nullptr);
    void asyncUpdate();
    org::freedesktop::portal::NetworkMonitor m_iface;
    NetworkStatus::State m_connected = NetworkStatus::Unknown;
    NetworkStatus::State m_metered = NetworkStatus::Unknown;
};
}

using namespace SolidExtras;

PortalNetworkMonitor::PortalNetworkMonitor(QObject *parent)
    : QObject(parent)
    , m_iface(QLatin1String("org.freedesktop.portal.Desktop"), QLatin1String("/org/freedesktop/portal/desktop"), QDBusConnection::sessionBus())
{
    connect(&m_iface, &org::freedesktop::portal::NetworkMonitor::changed, this, &PortalNetworkMonitor::asyncUpdate);
    if (m_iface.isValid()) {
        asyncUpdate();
    }
}

PortalNetworkMonitor* PortalNetworkMonitor::instance()
{
    static PortalNetworkMonitor *s_instance = new PortalNetworkMonitor(QCoreApplication::instance());
    return s_instance;
}

bool SolidExtras::PortalNetworkMonitor::hasPortal()
{
    return instance()->m_iface.isValid();
}

void PortalNetworkMonitor::asyncUpdate()
{
    auto connectivityPendingReply = m_iface.GetConnectivity();
    auto connectivityWatcher = new QDBusPendingCallWatcher(connectivityPendingReply, this);
    connect(connectivityWatcher, &QDBusPendingCallWatcher::finished, this, [this](auto *watcher) {
        const QDBusPendingReply<unsigned int> reply(*watcher);
        if (reply.isValid() && ((reply.value() == 4) != (m_connected == NetworkStatus::Yes) || m_connected == NetworkStatus::Unknown)) {
            m_connected = (reply.value() == 4) ? NetworkStatus::Yes : NetworkStatus::No;
            Q_EMIT connectivityChanged();
        }
    });

    auto meteredPendingReply = m_iface.GetMetered();
    auto meteredWatcher = new QDBusPendingCallWatcher(meteredPendingReply, this);
    connect(meteredWatcher, &QDBusPendingCallWatcher::finished, this, [this](auto *watcher) {
        const QDBusPendingReply<bool> reply(*watcher);
        if (reply.isValid() && (reply.value() != (m_metered == NetworkStatus::Yes) || m_metered == NetworkStatus::Unknown)) {
            m_metered = reply.value() ? NetworkStatus::Yes : NetworkStatus::No;
            Q_EMIT meteredChanged();
        }
    });
}


NetworkStatus::NetworkStatus(QObject *parent)
    : QObject(parent)
{
    qDebug() << "has portal:" << PortalNetworkMonitor::hasPortal();
    connect(PortalNetworkMonitor::instance(), &PortalNetworkMonitor::connectivityChanged, this, &NetworkStatus::connectivityChanged);
    connect(PortalNetworkMonitor::instance(), &PortalNetworkMonitor::meteredChanged, this, &NetworkStatus::meteredChanged);

#ifdef HAVE_NM
    if (!PortalNetworkMonitor::hasPortal()) {
        connect(NetworkManager::notifier(), &NetworkManager::Notifier::connectivityChanged, this, &NetworkStatus::connectivityChanged);
        connect(NetworkManager::notifier(), &NetworkManager::Notifier::meteredChanged, this, &NetworkStatus::meteredChanged);
    }
#endif
}

NetworkStatus::State NetworkStatus::connectivity() const
{
    if (PortalNetworkMonitor::hasPortal()) {
        return PortalNetworkMonitor::connectivity();
    }
#ifdef HAVE_NM
    switch (NetworkManager::connectivity()) {
        case NetworkManager::UnknownConnectivity:
            return Unknown;
        case NetworkManager::NoConnectivity:
        case NetworkManager::Portal:
        case NetworkManager::Limited:
            return No;
        case NetworkManager::Full:
            return Yes;
    }
    Q_UNREACHABLE();
#endif
    return NetworkStatus::Unknown;
}

NetworkStatus::State NetworkStatus::metered() const
{
    if (PortalNetworkMonitor::hasPortal()) {
        return PortalNetworkMonitor::metered();
    }
#ifdef HAVE_NM
    switch (NetworkManager::metered()) {
        case NetworkManager::Device::UnknownStatus:
            return Unknown;
        case NetworkManager::Device::GuessYes:
        case NetworkManager::Device::Yes:
            return Yes;
        case NetworkManager::Device::GuessNo:
        case NetworkManager::Device::No:
            return No;
    }
    Q_UNREACHABLE();
#endif
    return NetworkStatus::Unknown;
}

#include "networkstatus_dbus.moc"
