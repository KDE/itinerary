/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNIARRAY_H
#define KANDROIDEXTRAS_JNIARRAY_H

#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>

namespace KAndroidExtras {

namespace Internal {

/** Meta function for retrieving a JNI array .*/
template <typename Container, typename Value> struct FromArray {};

template <typename Container>
struct FromArray<Container, QAndroidJniObject>
{
    inline auto operator()(const QAndroidJniObject &array) const
    {
        if (!array.isValid()) {
            return Container{};
        }
        const auto a = static_cast<jobjectArray>(array.object());
        QAndroidJniEnvironment env;
        const auto size = env->GetArrayLength(a);
        Container r;
        r.reserve(size);
        for (auto i = 0; i < size; ++i) {
            r.push_back(QAndroidJniObject::fromLocalRef(env->GetObjectArrayElement(a, i)));
        }
        return r;
    }
};

template <typename Container>
struct FromArray<Container, QString>
{
    inline auto operator()(const QAndroidJniObject &array) const
    {
        if (!array.isValid()) {
            return Container{};
        }
        const auto a = static_cast<jobjectArray>(array.object());
        QAndroidJniEnvironment env;
        const auto size = env->GetArrayLength(a);
        Container r;
        r.reserve(size);
        for (auto i = 0; i < size; ++i) {
            r.push_back(QAndroidJniObject::fromLocalRef(env->GetObjectArrayElement(a, i)).toString());
        }
        return r;
    }
};

// TODO specializations for basic types

}

namespace Jni {
    /** Convert a JNI array to a C++ container.
     *  Container value types can be any of QAndroidJniObject, QString or a basic JNI type.
     */
    template <typename Container> constexpr __attribute__((__unused__)) Internal::FromArray<Container, typename Container::value_type> fromArray = {};
}

}

#endif
