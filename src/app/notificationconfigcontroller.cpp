/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

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
#if defined(Q_OS_ANDROID) && KNOTIFICATIONS_VERSION > QT_VERSION_CHECK(5, 70, 0) // TODO this needs unmerged patches still
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

