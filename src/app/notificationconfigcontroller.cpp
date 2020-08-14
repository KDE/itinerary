/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "notificationconfigcontroller.h"

#include <knotifications_version.h>

#ifdef Q_OS_ANDROID
#include <kandroidextras/activity.h>
#include <kandroidextras/context.h>
#include <kandroidextras/intent.h>
#include <kandroidextras/settings.h>
#endif

#include <QDebug>

bool NotificationConfigController::canConfigureNotification() const
{
#ifdef Q_OS_ANDROID
    return true;
#else
    return false; // TODO
#endif
}

bool NotificationConfigController::canShowOnLockScreen() const
{
#if defined(Q_OS_ANDROID) && KNOTIFICATIONS_VERSION >= QT_VERSION_CHECK(5, 71, 0)
    return true;
#else
    return false;
#endif
}

void NotificationConfigController::configureNotifications()
{
#ifdef Q_OS_ANDROID
    using namespace KAndroidExtras;

    Intent intent;
    intent.setAction(Settings::ACTION_APP_NOTIFICATION_SETTINGS);
    intent.putExtra<java::lang::String>(Settings::EXTRA_APP_PACKAGE, Context::getPackageName());
    Activity::startActivity(intent, 0);
#else
    // TODO
#endif
}

