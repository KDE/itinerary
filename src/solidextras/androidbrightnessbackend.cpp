/*
    SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "androidbrightnessbackend.h"

#include <QDebug>

#include <QCoreApplication>
#include <QJniObject>

AndroidBrightnessBackend::AndroidBrightnessBackend(QObject *parent)
    : BrightnessBackend(parent)
{
}

AndroidBrightnessBackend::~AndroidBrightnessBackend()
{
}

float AndroidBrightnessBackend::brightness() const
{
    return QJniObject::callStaticMethod<jfloat>("org.kde.solidextras.Solid",
                                                "getBrightness",
                                                "(Landroid/app/Activity;)F",
                                                QNativeInterface::QAndroidApplication::context());
}

void AndroidBrightnessBackend::setBrightness(float brightness)
{
    QJniObject::callStaticMethod<void>("org.kde.solidextras.Solid",
                                       "setBrightness",
                                       "(Landroid/app/Activity;F)V",
                                       QNativeInterface::QAndroidApplication::context(),
                                       brightness);
}

float AndroidBrightnessBackend::maxBrightness() const
{
    return 1;
}
