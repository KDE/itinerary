/*
    SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "androidlockbackend.h"

#include <QDebug>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtAndroid>
#include <QAndroidJniObject>
#else
#include <QCoreApplication>
#include <QJniObject>
using QAndroidJniObject = QJniObject;
#endif

AndroidLockBackend::AndroidLockBackend(QObject *parent)
    : LockBackend(parent)
{
}

AndroidLockBackend::~AndroidLockBackend()
{
}

void AndroidLockBackend::setInhibitionOff()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const auto activity = QtAndroid::androidActivity();
#else
    const QJniObject activity = QNativeInterface::QAndroidApplication::context();
#endif
    QAndroidJniObject::callStaticMethod<void>("org.kde.solidextras.Solid", "setLockInhibitionOff", "(Landroid/app/Activity;)V", activity.object());
}

void AndroidLockBackend::setInhibitionOn(const QString &explanation)
{
    Q_UNUSED(explanation)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const auto activity = QtAndroid::androidActivity();
#else
    const QJniObject activity = QNativeInterface::QAndroidApplication::context();
#endif
    QAndroidJniObject::callStaticMethod<void>("org.kde.solidextras.Solid", "setLockInhibitionOn", "(Landroid/app/Activity;)V", activity.object());
}
