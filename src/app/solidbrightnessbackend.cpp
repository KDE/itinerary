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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

void SolidBrightnessBackend::maxBrightness()
{
    m_iface->setBrightnessSilent(m_iface->brightnessMax());
}
