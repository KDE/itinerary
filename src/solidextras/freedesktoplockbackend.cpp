/*
    SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "freedesktoplockbackend.h"

#include "screensaverdbusinterface.h"

#include <QDBusConnection>

FreedesktopLockBackend::FreedesktopLockBackend(QObject *parent)
    : LockBackend(parent)
    , m_cookie(0)
{
    m_iface = new OrgFreedesktopScreenSaverInterface(QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("/org/freedesktop/ScreenSaver"), QDBusConnection::sessionBus(), this);
}

void FreedesktopLockBackend::setInhibitionOff()
{
    m_iface->UnInhibit(m_cookie);
}

void FreedesktopLockBackend::setInhibitionOn(const QString &explanation)
{
    m_cookie = m_iface->Inhibit(QStringLiteral("org.kde.itinerary"), explanation);
}

