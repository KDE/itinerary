/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "context.h"

#include <KAndroidExtras/JavaTypes>
#include <KAndroidExtras/JniSignature>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtAndroid>
#else
#include <QCoreApplication>
#endif

using namespace KAndroidExtras;

QAndroidJniObject Context::getPackageName()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const auto context = QtAndroid::androidContext();
#else
    const QJniObject context = QNativeInterface::QAndroidApplication::context();
#endif
    return context.callObjectMethod("getPackageName", Jni::signature<java::lang::String()>());
}
