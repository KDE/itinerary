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
