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
#include <QProcess>

using namespace Qt::Literals;

bool NotificationConfigController::canConfigureNotification() const
{
#ifdef Q_OS_ANDROID
    return true;
#else
    static bool s_hasNotificationKCM = []{
        QProcess proc;
        proc.start(u"kcmshell6"_s, {u"--list"_s});
        proc.waitForFinished();
        return proc.readAllStandardOutput().contains("\nkcm_notifications "_L1);
    }();

    return s_hasNotificationKCM;
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
    QProcess proc;
    proc.setProgram(u"kcmshell6"_s);
    proc.setArguments({u"--args"_s, "--notifyrc %1"_L1.arg(QCoreApplication::applicationName()), u"notifications"_s});
    proc.startDetached();
#endif
}

#include "moc_notificationconfigcontroller.cpp"
