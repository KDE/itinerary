/*
    SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "androidbrightnessbackend.h"

#include <QDebug>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtAndroid>
#include <QAndroidJniObject>
#else
#include <QCoreApplication>
#include <QJniObject>
using QAndroidJniObject = QJniObject;
#endif

AndroidBrightnessBackend::AndroidBrightnessBackend(QObject *parent)
    : BrightnessBackend(parent)
{
}

AndroidBrightnessBackend::~AndroidBrightnessBackend()
{
}

float AndroidBrightnessBackend::brightness() const
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const auto activity = QtAndroid::androidActivity();
#else
    const QJniObject activity = QNativeInterface::QAndroidApplication::context();
#endif
    return QAndroidJniObject::callStaticMethod<jfloat>("org.kde.solidextras.Solid", "getBrightness", "(Landroid/app/Activity;)F", activity.object());
}

void AndroidBrightnessBackend::setBrightness(float brightness)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const auto activity = QtAndroid::androidActivity();
#else
    const QJniObject activity = QNativeInterface::QAndroidApplication::context();
#endif
    QAndroidJniObject::callStaticMethod<void>("org.kde.solidextras.Solid", "setBrightness", "(Landroid/app/Activity;F)V", activity.object(), brightness);
}

float AndroidBrightnessBackend::maxBrightness() const
{
    return 1;
}
