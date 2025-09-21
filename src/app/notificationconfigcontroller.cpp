/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "notificationconfigcontroller.h"

#ifdef Q_OS_ANDROID
#include "kandroidextras/activity.h"
#include "kandroidextras/context.h"
#include "kandroidextras/intent.h"
#include "kandroidextras/settings.h"
#endif

#include <QCoreApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

using namespace Qt::Literals;

bool NotificationConfigController::canConfigureNotification() const
{
#ifdef Q_OS_ANDROID
    return true;
#else
    return qgetenv("XDG_CURRENT_DESKTOP") == "KDE";
#endif
}

bool NotificationConfigController::canShowOnLockScreen() const
{
#if defined(Q_OS_ANDROID)
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
    QUrl url;
    url.setScheme(u"systemsettings"_s);
    url.setHost(u"kcm_notifications"_s);
    url.setPath("/--notifyrc "_L1 + QCoreApplication::applicationName());
    QDesktopServices::openUrl(url);
#endif
}

#include "moc_notificationconfigcontroller.cpp"
