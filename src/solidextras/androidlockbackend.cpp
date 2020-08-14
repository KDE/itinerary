/*
    SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "androidlockbackend.h"

#include <QDebug>
#include <QtAndroid>
#include <QAndroidJniObject>

AndroidLockBackend::AndroidLockBackend(QObject *parent)
    : LockBackend(parent)
{
}

AndroidLockBackend::~AndroidLockBackend()
{
}

void AndroidLockBackend::setInhibitionOff()
{
    QAndroidJniObject::callStaticMethod<void>("org.kde.solid.Solid", "setLockInhibitionOff", "(Landroid/app/Activity;)V", QtAndroid::androidActivity().object());
}

void AndroidLockBackend::setInhibitionOn(const QString &explanation)
{
    Q_UNUSED(explanation);
    QAndroidJniObject::callStaticMethod<void>("org.kde.solid.Solid", "setLockInhibitionOn", "(Landroid/app/Activity;)V", QtAndroid::androidActivity().object());
}
