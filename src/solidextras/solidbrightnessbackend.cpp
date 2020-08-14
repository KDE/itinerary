/*
    SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "solidbrightnessbackend.h"

#include <brightnesscontroldbusinterface.h>
#include <QDBusConnection>
#include <QDebug>

SolidBrightnessBackend::SolidBrightnessBackend(QObject *parent)
    : BrightnessBackend(parent)
{
    m_iface = new OrgKdeSolidPowerManagementActionsBrightnessControlInterface(
            QStringLiteral("org.kde.Solid.PowerManagement"),
            QStringLiteral("/org/kde/Solid/PowerManagement/Actions/BrightnessControl"),
            QDBusConnection::sessionBus(), this);
}

SolidBrightnessBackend::~SolidBrightnessBackend()
{
}

float SolidBrightnessBackend::brightness() const
{
    return m_iface->brightness();
}

void SolidBrightnessBackend::setBrightness(float brightness)
{
    m_iface->setBrightnessSilent(brightness);
}

float SolidBrightnessBackend::maxBrightness() const
{
    return m_iface->brightnessMax();
}
