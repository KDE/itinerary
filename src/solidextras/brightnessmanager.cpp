/*
    SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "brightnessmanager.h"

#include <QDebug>

#if defined(Q_OS_ANDROID)
#include "androidbrightnessbackend.h"
#elif defined(Q_OS_LINUX)
#include "solidbrightnessbackend.h"
#endif

BrightnessManager::BrightnessManager(QObject *parent)
    : QObject(parent)
{
#if defined(Q_OS_ANDROID)
    m_backend = new AndroidBrightnessBackend(this);
#elif defined(Q_OS_LINUX)
    m_backend = new SolidBrightnessBackend(this);
#endif
}

BrightnessManager::~BrightnessManager() = default;

void BrightnessManager::toggleBrightness()
{
    if (!m_backend)
        return;

    m_backend->toggleBrightness();
}

void BrightnessBackend::toggleBrightness()
{
    if (m_maximized) {
        setBrightness(m_previousValue);
    } else {
        m_previousValue = brightness();
        setBrightness(maxBrightness());
    }
    m_maximized = !m_maximized;
}

#include "moc_brightnessmanager.cpp"
