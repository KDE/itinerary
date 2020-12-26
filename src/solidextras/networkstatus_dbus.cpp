/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-solid-extras.h"
#include "networkstatus.h"

#ifdef HAVE_NM
#include <NetworkManagerQt/Manager>
#endif

using namespace SolidExtras;

NetworkStatus::NetworkStatus(QObject *parent)
    : QObject(parent)
{
#ifdef HAVE_NM
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::connectivityChanged, this, &NetworkStatus::connectivityChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::meteredChanged, this, &NetworkStatus::meteredChanged);
#endif
}

NetworkStatus::State NetworkStatus::connectivity() const
{
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
