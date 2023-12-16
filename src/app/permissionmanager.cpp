/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "permissionmanager.h"

#include <QDebug>

#ifdef Q_OS_ANDROID
#include <KAndroidExtras/ManifestPermission>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtAndroid>
#else
#include <private/qandroidextras_p.h>
#endif
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
        case Permission::PostNotification:  // only for SDK >= 33
            return ManifestPermission::POST_NOTIFICATIONS;
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (permission == Permission::PostNotification && QtAndroid::androidSdkVersion() < 33) {
        return true;
    }

    return QtAndroid::checkPermission(permissionName(permission)) == QtAndroid::PermissionResult::Granted;
#else
    if (permission == Permission::PostNotification && QtAndroidPrivate::androidSdkVersion() < 33) {
        return true;
    }

    return QtAndroidPrivate::checkPermission(permissionName(permission)).result() == QtAndroidPrivate::PermissionResult::Authorized;
#endif
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QtAndroid::requestPermissions({permissionName(permission)}, [permission, callback] (const QtAndroid::PermissionResultMap &result) {
        if (result[permissionName(permission)] == QtAndroid::PermissionResult::Granted) {
            auto cb = callback;
            cb.call();
        }
    });
#else
    // TODO make this properly async
    if (QtAndroidPrivate::checkPermission(permissionName(permission)).result() != QtAndroidPrivate::PermissionResult::Authorized) {
        if (QtAndroidPrivate::requestPermission(permissionName(permission)).result() != QtAndroidPrivate::PermissionResult::Authorized) {
            return;
        }
    }
    callback.call();
#endif
#endif
}

#include "moc_permissionmanager.cpp"
