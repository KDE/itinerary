/*
    Copyright (C) 2019 Nicolas Fella <nicolas.fella@gmx.de>

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

#include "solidlockbackend.h"

#include "screensaverdbusinterface.h"

#include <QDBusConnection>

SolidLockBackend::SolidLockBackend(QObject *parent)
    : LockBackend(parent)
    , m_cookie(0)
{
    m_iface = new OrgFreedesktopScreenSaverInterface(QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("/org/freedesktop/ScreenSaver"), QDBusConnection::sessionBus(), this);
}

void SolidLockBackend::setInhibitionOff()
{
    m_iface->UnInhibit(m_cookie);
}

void SolidLockBackend::setInhibitionOn(const QString &explanation)
{
    m_cookie = m_iface->Inhibit(QStringLiteral("org.kde.itinerary"), explanation);
}

