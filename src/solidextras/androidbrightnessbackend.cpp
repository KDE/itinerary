/*
    SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "androidbrightnessbackend.h"

#include <QDebug>
#include <QtAndroid>
#include <QAndroidJniObject>

AndroidBrightnessBackend::AndroidBrightnessBackend(QObject *parent)
    : BrightnessBackend(parent)
{
}

AndroidBrightnessBackend::~AndroidBrightnessBackend()
{
}

float AndroidBrightnessBackend::brightness() const
{
    return QAndroidJniObject::callStaticMethod<jfloat>("org.kde.solidextras.Solid", "getBrightness", "(Landroid/app/Activity;)F", QtAndroid::androidActivity().object());
}

void AndroidBrightnessBackend::setBrightness(float brightness)
{
    QAndroidJniObject::callStaticMethod<void>("org.kde.solidextras.Solid", "setBrightness", "(Landroid/app/Activity;F)V", QtAndroid::androidActivity().object(), brightness);
}

float AndroidBrightnessBackend::maxBrightness() const
{
    return 1;
}
