/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "activity.h"

#include <KAndroidExtras/AndroidTypes>
#include <KAndroidExtras/Intent>
#include <KAndroidExtras/JniSignature>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtAndroid>
#include <QAndroidJniEnvironment>
#else
#include <QCoreApplication>
#include <QJniEnvironment>
using QJniEnvironment = QAndroidJniEnvironment;

#include <private/qandroidextras_p.h>
#endif

using namespace KAndroidExtras;

Intent Activity::getIntent()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const auto activity = QtAndroid::androidActivity();
#else
    const QJniObject activity = QNativeInterface::QAndroidApplication::context();
#endif

    if (!activity.isValid())
        return {};

    const auto intent = activity.callObjectMethod("getIntent", Jni::signature<android::content::Intent()>());
    return Intent(Jni::fromHandle<Intent>(intent));
}

bool Activity::startActivity(const Intent &intent, int receiverRequestCode)
{
    QAndroidJniEnvironment jniEnv;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QtAndroid::startActivity(intent, receiverRequestCode);
#else
    QtAndroidPrivate::startActivity(intent, receiverRequestCode);
#endif
    if (jniEnv->ExceptionCheck()) {
        jniEnv->ExceptionClear();
        return false;
    }
    return true;
}
