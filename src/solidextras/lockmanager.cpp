/*
    SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
