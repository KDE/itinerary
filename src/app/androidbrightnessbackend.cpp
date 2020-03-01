/*
    Copyright (C) 2018 Nicolas Fella <nicolas.fella@gmx.de>

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
    return QAndroidJniObject::callStaticMethod<jfloat>("org.kde.solid.Solid", "getBrightness", "(Landroid/app/Activity;)F", QtAndroid::androidActivity().object());
}

void AndroidBrightnessBackend::setBrightness(float brightness)
{
    QAndroidJniObject::callStaticMethod<void>("org.kde.solid.Solid", "setBrightness", "(Landroid/app/Activity;F)V", QtAndroid::androidActivity().object(), brightness);
}

float AndroidBrightnessBackend::maxBrightness() const
{
    return 1;
}
