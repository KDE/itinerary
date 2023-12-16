/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PERMISSIONMANAGER_H
#define PERMISSIONMANAGER_H

#include <QJSValue>

/** Permission enum for use in QML. */
namespace Permission
{
    Q_NAMESPACE
    enum Permission {
        InvalidPermission, // captures QML errors resultin in "0" enum values
        ReadCalendar,
        WriteCalendar,
        PostNotification,
        Camera,
    };
    Q_ENUM_NS(Permission)
}

/** Check and request platform permissions for access to controlled resources (calendar, location, etc). */
class PermissionManager
{
    Q_GADGET
public:
    Q_INVOKABLE static bool checkPermission(Permission::Permission permission);
    Q_INVOKABLE static void requestPermission(Permission::Permission permission, QJSValue callback);
};

#endif // PERMISSIONMANAGER_H
