/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNIARRAY_H
#define KANDROIDEXTRAS_JNIARRAY_H

#include "jnitypetraits.h"

#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>

namespace KAndroidExtras {

///@cond internal
namespace Internal {

/** Basic type array type traits. */
template <typename T> struct array_trait {};

#define MAKE_ARRAY_TRAIT(base_type, type_name) \
template <> struct array_trait<base_type> { \
    typedef base_type ## Array type; \
    static inline type newArray(JNIEnv *env, jsize size) { return env->New ## type_name ## Array(size); } \
    static inline base_type* getArrayElements(JNIEnv *env, type array, jboolean *isCopy) { return env->Get ## type_name ## ArrayElements(array, isCopy); } \
    static inline void releaseArrayElements(JNIEnv *env, type array, base_type *data, jint mode) { return env->Release ## type_name ## ArrayElements(array, data, mode); } \
    static inline void setArrayRegion(JNIEnv *env, type array, jsize start, jsize length, const base_type *data) { env->Set ## type_name ## ArrayRegion(array, start, length, data); } \
};

MAKE_ARRAY_TRAIT(jboolean, Boolean)
MAKE_ARRAY_TRAIT(jbyte, Byte)
MAKE_ARRAY_TRAIT(jchar, Char)
MAKE_ARRAY_TRAIT(jshort, Short)
MAKE_ARRAY_TRAIT(jint, Int)
MAKE_ARRAY_TRAIT(jlong, Long)
MAKE_ARRAY_TRAIT(jfloat, Float)
MAKE_ARRAY_TRAIT(jdouble, Double)

#undef MAKE_ARRAY_TRAIT

/** Meta function for retrieving a JNI array .*/
template <typename Container, typename Value, bool is_basic> struct FromArray {};

template <typename Container>
struct FromArray<Container, QAndroidJniObject, false>
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
struct FromArray<Container, QString, false>
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

// specializations for basic types
template <typename Container, typename Value>
struct FromArray<Container, Value, true>
{
    typedef array_trait<Value> _t;
    inline auto operator()(const QAndroidJniObject &array) const
    {
        if (!array.isValid()) {
            return Container{};
        }

        const auto a = static_cast<typename _t::type>(array.object());
        QAndroidJniEnvironment env;
        const auto size = env->GetArrayLength(a);
        Container r;
        r.reserve(size);

        auto data = _t::getArrayElements(env, a, nullptr);
        std::copy(data, data + size, std::back_inserter(r));
        _t::releaseArrayElements(env, a, data, JNI_ABORT);

        return r;
    }
};

}
///@endcond

namespace Jni {
    /** Convert a JNI array to a C++ container.
     *  Container value types can be any of QAndroidJniObject, QString or a basic JNI type.
     */
    template <typename Container> constexpr __attribute__((__unused__)) Internal::FromArray<Container, typename Container::value_type, Jni::is_basic_type<typename Container::value_type>::value> fromArray = {};
}

}

#endif
