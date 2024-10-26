/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "permissionmanager.h"

#include <QDebug>

#ifdef Q_OS_ANDROID
#include "private/qandroidextras_p.h"
#include <KAndroidExtras/ManifestPermission>
#endif

#ifdef Q_OS_ANDROID
static QString permissionName(Permission::Permission p)
{
    using namespace KAndroidExtras;
    switch (p) {
    case Permission::InvalidPermission:
        Q_UNREACHABLE();
    case Permission::ReadCalendar:
        return ManifestPermission::READ_CALENDAR;
    case Permission::WriteCalendar:
        return ManifestPermission::WRITE_CALENDAR;
    case Permission::Camera:
        return ManifestPermission::CAMERA;
    }
}
#endif

bool PermissionManager::checkPermission(Permission::Permission permission)
{
    if (permission == Permission::InvalidPermission) {
        qWarning() << "check for invalid permission - check your QML code!";
        return false;
    }
#ifdef Q_OS_ANDROID
    return QtAndroidPrivate::checkPermission(permissionName(permission)).result() == QtAndroidPrivate::PermissionResult::Authorized;
#else // non-Android
    Q_UNUSED(permission);
    return true;
#endif
}

void PermissionManager::requestPermission(Permission::Permission permission, QJSValue callback)
{
    if (permission == Permission::InvalidPermission) {
        qWarning() << "request for invalid permission - check your QML code!";
        return;
    }

    qDebug() << permission;
    // already got the permission
    if (PermissionManager::checkPermission(permission)) {
        callback.call();
        return;
    }

#ifdef Q_OS_ANDROID
    // TODO make this properly async
    if (QtAndroidPrivate::requestPermission(permissionName(permission)).result() != QtAndroidPrivate::PermissionResult::Authorized) {
        return;
    }
    // special case for WriteCalendar which practically depends on ReadCalendar
    if (permission == Permission::WriteCalendar && !PermissionManager::checkPermission(Permission::ReadCalendar)) {
        if (QtAndroidPrivate::requestPermission(permissionName(Permission::ReadCalendar)).result() != QtAndroidPrivate::PermissionResult::Authorized) {
            return;
        }
    }

    callback.call();
#endif
}

#include "moc_permissionmanager.cpp"
