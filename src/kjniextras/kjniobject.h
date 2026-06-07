/*
    SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KJNIEXTRAS_JNIOBJECT_H
#define KJNIEXTRAS_JNIOBJECT_H

#include "kjnipp.h"

#include <QJniObject>
#include <QtJniTypes>

/** Annotates a class for holding JNI method or property wrappers.
 *
 *  @param ThisType the name of the class this is added to. Has to be dereived from QtJniTypes::JObject<T>.
 *
 *  @see KJNI_DECLARE_CLASS
 */
#define KJNI_OBJECT(ThisType) \
    typedef ThisType _kjni_ThisType; \
public: \
    using JObject::JObject; \
private:

///@cond internal
#define KJNI_DECLARE_CLASS_1(Name, Type, ...) \
template <> struct ::QtJniTypes::Traits<Type> { \
    static constexpr auto className() { return QtJniTypes::CTString(Name); } \
    static constexpr auto signature() { return QtJniTypes::CTString("L") + className() + QtJniTypes::CTString(";"); } \
    static auto convertToJni(JNIEnv *, const Type &value) { return value.object(); } \
    static auto convertFromJni(QJniObject &&object) { return Type(std::move(object)); } \
};

#define KJNI_DECLARE_CLASS_2(N1, N2, ...) \
    KJNI_DECLARE_CLASS_1(N1 #N2, __VA_ARGS__)
#define KJNI_DECLARE_CLASS_3(N1, N2, ...) \
    KJNI_DECLARE_CLASS_2(N1 #N2 "/" , __VA_ARGS__)
#define KJNI_DECLARE_CLASS_4(N1, N2, ...) \
    KJNI_DECLARE_CLASS_3(N1 #N2 "/" , __VA_ARGS__)
#define KJNI_DECLARE_CLASS_5(N1, N2, ...) \
    KJNI_DECLARE_CLASS_4(N1 #N2 "/" , __VA_ARGS__)
#define KJNI_DECLARE_CLASS_6(N1, N2, ...) \
    KJNI_DECLARE_CLASS_5(N1 #N2 "/" , __VA_ARGS__)

#define KJNI_DECLARE_CLASS_(N, name, ...) KJNI_PP_CONCAT(KJNI_DECLARE_CLASS_, N)(name, __VA_ARGS__)
///@endcond

/** Create Qt JNI type traits for @p Type.
 *  Must be called after declaring a class containing @c KJNI_OBJECT.
 */
#define KJNI_DECLARE_CLASS(...) KJNI_DECLARE_CLASS_(KJNI_PP_NARG(__VA_ARGS__), "", __VA_ARGS__)


///@cond internal
#define KJNI_DECLARE_NESTED_CLASS_2(Name, NestedName, Type, ...) \
template <> struct ::QtJniTypes::Traits<Type> { \
    static constexpr auto className() { return QtJniTypes::CTString(Name "$" #NestedName); } \
    static constexpr auto signature() { return QtJniTypes::CTString("L") + className() + QtJniTypes::CTString(";"); } \
    static auto convertToJni(JNIEnv *, const Type &value) { return value.object(); } \
    static auto convertFromJni(QJniObject &&object) { return Type(std::move(object)); } \
};

#define KJNI_DECLARE_NESTED_CLASS_3(N1, N2, ...) \
    KJNI_DECLARE_NESTED_CLASS_2(N1 #N2, __VA_ARGS__)
#define KJNI_DECLARE_NESTED_CLASS_4(N1, N2, ...) \
    KJNI_DECLARE_NESTED_CLASS_3(N1 #N2 "/", __VA_ARGS__)
#define KJNI_DECLARE_NESTED_CLASS_5(N1, N2, ...) \
    KJNI_DECLARE_NESTED_CLASS_4(N1 #N2 "/", __VA_ARGS__)
#define KJNI_DECLARE_NESTED_CLASS_6(N1, N2, ...) \
    KJNI_DECLARE_NESTED_CLASS_5(N1 #N2 "/", __VA_ARGS__)
#define KJNI_DECLARE_NESTED_CLASS_7(N1, N2, ...) \
    KJNI_DECLARE_NESTED_CLASS_6(N1 #N2 "/", __VA_ARGS__)

#define KJNI_DECLARE_NESTED_CLASS_(N, name, ...) KJNI_PP_CONCAT(KJNI_DECLARE_NESTED_CLASS_, N)(name, __VA_ARGS__)
///@endcond

/** Same as KJNI_DECLARE_CLASS, but for nested Java classes. */
#define KJNI_DECLARE_NESTED_CLASS(...) KJNI_DECLARE_NESTED_CLASS_(KJNI_PP_NARG(__VA_ARGS__), "", __VA_ARGS__)

#endif

