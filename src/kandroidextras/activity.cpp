/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "activity.h"
#include "intent.h"
#include "jnisignature.h"
#include "jnitypes.h"

#include <QtAndroid>
#include <QAndroidJniEnvironment>

using namespace KAndroidExtras;

Intent Activity::getIntent()
{
    const auto activity = QtAndroid::androidActivity();
    if (!activity.isValid())
        return {};

    const auto intent = activity.callObjectMethod("getIntent", Jni::signature<android::content::Intent()>());
    return Intent(intent);
}

bool Activity::startActivity(const Intent &intent, int receiverRequestCode)
{
    QAndroidJniEnvironment jniEnv;
    QtAndroid::startActivity(intent, receiverRequestCode);
    if (jniEnv->ExceptionCheck()) {
        jniEnv->ExceptionClear();
        return false;
    }
    return true;
}
