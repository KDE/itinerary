/*
    SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "androidlockbackend.h"

#include <QCoreApplication>
#include <QJniObject>

AndroidLockBackend::AndroidLockBackend(QObject *parent)
    : LockBackend(parent)
{
}

AndroidLockBackend::~AndroidLockBackend()
{
}

void AndroidLockBackend::setInhibitionOff()
{
    QJniObject::callStaticMethod<void>("org.kde.solidextras.Solid",
                                       "setLockInhibitionOff",
                                       "(Landroid/app/Activity;)V",
                                       QNativeInterface::QAndroidApplication::context());
}

void AndroidLockBackend::setInhibitionOn(const QString &explanation)
{
    Q_UNUSED(explanation)
    QJniObject::callStaticMethod<void>("org.kde.solidextras.Solid",
                                       "setLockInhibitionOn",
                                       "(Landroid/app/Activity;)V",
                                       QNativeInterface::QAndroidApplication::context());
}
