/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "notificationconfigcontroller.h"

using namespace Qt::Literals;

bool NotificationConfigController::canShowOnLockScreen()
{
#if defined(Q_OS_ANDROID)
    return true;
#else
    return false;
#endif
}

#include "moc_notificationconfigcontroller.cpp"
