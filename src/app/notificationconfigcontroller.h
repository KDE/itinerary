/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef NOTIFICATIONCONFIGCONTROLLER_H
#define NOTIFICATIONCONFIGCONTROLLER_H

#include <QMetaType>

/** QML interface for notification configuration. */
class NotificationConfigController
{
    Q_GADGET
    Q_PROPERTY(bool canConfigureNotification READ canConfigureNotification)
    Q_PROPERTY(bool canShowOnLockScreen READ canShowOnLockScreen)
public:
    /** Notification configuration dialog is available on this platform. */
    bool canConfigureNotification() const;
    /** Platform supports lock-screen visibility. */
    bool canShowOnLockScreen() const;

    /** Show notification config dialog. */
    Q_INVOKABLE void configureNotifications();
};

Q_DECLARE_METATYPE(NotificationConfigController)

#endif // NOTIFICATIONCONFIGCONTROLLER_H
