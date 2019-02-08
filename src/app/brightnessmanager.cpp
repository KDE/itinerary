/*
    Copyright (C) 2018 Nicolas Fella <nicolas.fella@gmx.de>

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
