/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "networkstatus.h"

using namespace SolidExtras;

NetworkStatus::NetworkStatus(QObject *parent)
    : QObject(parent)
{
}

NetworkStatus::State NetworkStatus::connectivity() const
{
    return NetworkStatus::Unknown;
}

NetworkStatus::State NetworkStatus::metered() const
{
    return NetworkStatus::Unknown;
}
