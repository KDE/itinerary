/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KJNIEXTRAS_JNIMETHOD_H
#define KJNIEXTRAS_JNIMETHOD_H

#include "kjnipp.h"

#include <utility>

///@cond internal

// method parameter generation
#define KJNI_PARAM(Type, Name) std::conditional_t<QtJniTypes::isPrimitiveType<Type>(), Type, const Type&> Name

#define KJNI_PARAMS_0(accu, arg)
#define KJNI_PARAMS_1(accu, arg, ...) KJNI_PARAM(arg, a1)
#define KJNI_PARAMS_2(accu, arg, ...) KJNI_PARAM(arg, a2), KJNI_PARAMS_1(accu, __VA_ARGS__)
#define KJNI_PARAMS_3(accu, arg, ...) KJNI_PARAM(arg, a3), KJNI_PARAMS_2(accu, __VA_ARGS__)

#define KJNI_PARAMS_(N, accu, ...) KJNI_PP_CONCAT(KJNI_PARAMS_, N)(accu, __VA_ARGS__)
#define KJNI_PARAMS(...) KJNI_PARAMS_(KJNI_PP_NARG(__VA_ARGS__), "", __VA_ARGS__)

// method argument forwarding generation
#define KJNI_ARG(Type, Name) Name
#define KJNI_ARGS_0(accu, arg)
#define KJNI_ARGS_1(accu, arg, ...) KJNI_ARG(arg, a1)
#define KJNI_ARGS_2(accu, arg, ...) KJNI_ARG(arg, a2), KJNI_ARGS_1(accu, __VA_ARGS__)
#define KJNI_ARGS_3(accu, arg, ...) KJNI_ARG(arg, a3), KJNI_ARGS_2(accu, __VA_ARGS__)

#define KJNI_ARGS_(N, accu, ...) KJNI_PP_CONCAT(KJNI_ARGS_, N)(accu, __VA_ARGS__)
#define KJNI_ARGS(...) KJNI_ARGS_(KJNI_PP_NARG(__VA_ARGS__), "", __VA_ARGS__)

///@endcond

/**
 * Wrap a JNI method call.
 * This will add a method named @p Name to the current class. Argument types are checked at compile time.
 *
 * @param RetT The return type. Must either be a primitive type or a type declared with @c Q_DECLARE_JNI_CLASS.
 * @param Name The name of the method. Must match the JNI method to be called exactly.
 * @param Args A list or argument types (can be empty). Must either be primitive types or types declared
 *        with @c Q_DECLARE_JNI_CLASS
 */
#define KJNI_METHOD(RetT, Name, ...) \
    inline auto Name(KJNI_PARAMS(__VA_ARGS__)) const \
    { \
        return callMethod<RetT>("" #Name __VA_OPT__(, ) KJNI_ARGS(__VA_ARGS__)); \
    }

/**
 * Wrap a JNI static method call.
 * This will add a static method named @p Name to the current class. Argument types are checked at compile time.
 *
 * This macro can only be placed in classes deriving from QtJniTypes::JObject<T>.
 *
 * @param RetT The return type. Must either be a primitive type or a type declared with @c JNI_TYPE
 * @param Name The name of the method. Must match the JNI method to be called exactly.
 * @param Args A list or argument types (can be empty). Must either be primitive types or types declared
 *        with @c JNI_TYPE.
 */
#define JNI_STATIC_METHOD(RetT, Name, ...) \
    static inline auto Name(KJNI_PARAMS(__VA_ARGS__)) \
    { \
        return callStaticMethod<RetT>("" #Name __VA_OPT__(, ) KJNI_ARGS(__VA_ARGS__)); \
    }

/**
 * Wrap a JNI constructor call.
 * This will add a constructor named @p Name to the current class. Argument types are checked at compile time.
 *
 * This macro can only be placed in classes deriving from QtJniTypes::JObject<T>.
 *
 * @param Name The name of the method. Must match the JNI method to be called exactly.
 * @param Args A list or argument types (can be empty). Must either be primitive types or types declared
 *        with @c Q_DECLARE_JNI_CLASS.
 */
#define KJNI_CONSTRUCTOR(Name, ...) \
    Name(KJNI_PARAMS(__VA_ARGS__)) : QtJniTypes::JObject<Class>(KJNI_ARGS(__VA_ARGS__)) {}

#endif
