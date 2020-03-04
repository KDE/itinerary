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

#include "lockmanager.h"

#include <QDebug>

#if defined(Q_OS_ANDROID)
#include "androidlockbackend.h"
#elif defined(Q_OS_LINUX)
#include "solidlockbackend.h"
#endif

LockManager::LockManager(QObject *parent)
    : QObject(parent)
    , m_inhibit()
{
#if defined(Q_OS_ANDROID)
    m_backend = new AndroidLockBackend(this);
#elif defined(Q_OS_LINUX)
    m_backend = new SolidLockBackend(this);
#endif
}

LockManager::~LockManager() = default;

void LockManager::toggleInhibitScreenLock(const QString &explanation)
{
    if (!m_backend)
        return;

    if (m_inhibit) {
        m_backend->setInhibitionOff();
    } else {
        m_backend->setInhibitionOn(explanation);
    }
    m_inhibit = !m_inhibit;
}
