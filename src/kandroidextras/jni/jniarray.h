/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNIARRAY_H
#define KANDROIDEXTRAS_JNIARRAY_H

#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>

namespace KAndroidExtras {

namespace Jni {
    /** Convert a java.lang.String[] to a QStringList.
     *  @todo generalize this to convert any kind of JNI array.
     */
    inline QStringList fromArray(const QAndroidJniObject &array)
    {
        if (!array.isValid()) {
            return {};
        }

        const auto a = static_cast<jobjectArray>(array.object());
        QAndroidJniEnvironment env;
        const auto size = env->GetArrayLength(a);
        QStringList r;
        r.reserve(size);
        for (auto i = 0; i < size; ++i) {
            r.push_back(QAndroidJniObject::fromLocalRef(env->GetObjectArrayElement(a, i)).toString());
        }
        return r;
    }
}

}

#endif
