/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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
