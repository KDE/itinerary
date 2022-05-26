/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNIMETHOD_H
#define KANDROIDEXTRAS_JNIMETHOD_H

#include "jniargument.h"
#include "jniarray.h"
#include "jniobject.h"
#include "jnipp.h"
#include "jnireturnvalue.h"
#include "jnitypetraits.h"

#include <QAndroidJniObject>

namespace KAndroidExtras {
///@cond internal

// method parameter generation
#define JNI_PARAM(Type, Name) typename KAndroidExtras::Internal::argument<Type>::type Name

#define JNI_PARAMS_0(accu, arg)
#define JNI_PARAMS_1(accu, arg, ...) JNI_PARAM(arg, a1)
#define JNI_PARAMS_2(accu, arg, ...) JNI_PARAM(arg, a2), JNI_PARAMS_1(accu, __VA_ARGS__)
#define JNI_PARAMS_3(accu, arg, ...) JNI_PARAM(arg, a3), JNI_PARAMS_2(accu, __VA_ARGS__)

#define JNI_PARAMS_(N, accu, ...) JNI_PP_CONCAT(JNI_PARAMS_, N)(accu, __VA_ARGS__)
#define JNI_PARAMS(...) JNI_PARAMS_(JNI_PP_NARG(__VA_ARGS__), "", __VA_ARGS__)

// method argument forwarding generation
#define JNI_ARG(Type, Name) KAndroidExtras::Internal::argument<Type>::toCallArgument(Name)
#define JNI_ARGS_0(accu, arg)
#define JNI_ARGS_1(accu, arg, ...) JNI_ARG(arg, a1)
#define JNI_ARGS_2(accu, arg, ...) JNI_ARG(arg, a2), JNI_ARGS_1(accu, __VA_ARGS__)
#define JNI_ARGS_3(accu, arg, ...) JNI_ARG(arg, a3), JNI_ARGS_2(accu, __VA_ARGS__)

#define JNI_ARGS_(N, accu, ...) JNI_PP_CONCAT(JNI_ARGS_, N)(accu, __VA_ARGS__)
#define JNI_ARGS(...) JNI_ARGS_(JNI_PP_NARG(__VA_ARGS__), "", __VA_ARGS__)

namespace Internal {
    // method call wrapper
    template <typename RetT>
    struct caller {
        template <typename ...Args>
        static auto call(const QAndroidJniObject &handle, const char *name, const char *signature, Args&&... args)
        {
            if constexpr (Jni::is_basic_type<RetT>::value) {
                return handle.callMethod<RetT>(name, signature, std::forward<Args>(args)...);
            } else {
                return Internal::return_wrapper<RetT>::toReturnValue(handle.callObjectMethod(name, signature, std::forward<Args>(args)...));
            }
        }

        static auto call(const QAndroidJniObject &handle, const char *name, const char *signature)
        {
            if constexpr (Jni::is_basic_type<RetT>::value) {
                return handle.callMethod<RetT>(name, signature);
            } else {
                return Internal::return_wrapper<RetT>::toReturnValue(handle.callObjectMethod(name, signature));
            }
        }
    };

    // static method call wrapper
    template <typename RetT>
    struct static_caller {
        template <typename ...Args>
        static auto call(const char *className, const char *name, const char *signature, Args&&... args)
        {
            if constexpr (Jni::is_basic_type<RetT>::value) {
                return QAndroidJniObject::callStaticMethod<RetT>(className, name, signature, std::forward<Args>(args)...);
            } else {
                return Internal::return_wrapper<RetT>::toReturnValue(QAndroidJniObject::callStaticObjectMethod(className, name, signature, std::forward<Args>(args)...));
            }
        }
        static auto call(const char *className, const char *name, const char *signature)
        {
            if constexpr (Jni::is_basic_type<RetT>::value) {
                return QAndroidJniObject::callStaticMethod<RetT>(className, name, signature);
            } else {
                return Internal::return_wrapper<RetT>::toReturnValue(QAndroidJniObject::callStaticObjectMethod(className, name, signature));
            }
        }
    };
}
///@endcond

/**
 * Wrap a JNI method call.
 * This will add a method named @p Name to the current class. Argument types are checked at compile time,
 * with the following inputs being accepted:
 * - basic types have to match exactly
 * - non-basic types can be either passed as @c QAndroidJniObject instance or with a type that has an
 *   conversion registered with @c JNI_DECLARE_CONVERTER.
 *
 * The return type of the method is determined as follows:
 * - basic types are returned directly
 * - non-basic types without a registered type conversion are returned as @c QAndroidJniObject.
 * - non-basic types with a registered type conversion are returned in a wrapper class that can
 *   be implicitly converted either to the destination type of the conversion, or a @c QAndroidJniObject.
 *   This allows to avoid type conversion when chaining calls for example, it however needs additional
 *   care when used in combination with automatic type deduction.
 * - array return types also result in a wrapper class that can be implicitly converted to a sequential
 *   container or a @p QAndroidJniObject representing the JNI array.
 *
 * Thie macro can only be placed in classes having the @c JNI_OBJECT macro.
 *
 * @param RetT The return type. Must either be a basic type or a type declared with @c JNI_TYPE
 * @param Name The name of the method. Must match the JNI method to be called exactly.
 * @param Args A list or argument types (can be empty). Must either be basic types or types declared
 *        with @c JNI_TYPE.
 */
#define JNI_METHOD(RetT, Name, ...) \
inline auto Name( JNI_PARAMS(__VA_ARGS__) ) const { \
    using namespace KAndroidExtras; \
    return Internal::caller<RetT>::call(jniHandle(), "" #Name, Jni::signature<RetT(__VA_ARGS__)>() __VA_OPT__(,) JNI_ARGS(__VA_ARGS__)); \
}

/**
 * Wrap a JNI static method call.
 * This will add a static method named @p Name to the current class. Argument types are checked at compile time,
 * with the following inputs being accepted:
 * - basic types have to match exactly
 * - non-basic types can be either passed as @c QAndroidJniObject instance or with a type that has an
 *   conversion registered with @c JNI_DECLARE_CONVERTER.
 *
 * The return type of the method is determined as follows:
 * - basic types are returned directly
 * - non-basic types without a registered type conversion are returned as @c QAndroidJniObject.
 * - non-basic types with a registered type conversion are returned in a wrapper class that can
 *   be implicitly converted either to the destination type of the conversion, or a @c QAndroidJniObject.
 *   This allows to avoid type conversion when chaining calls for example, it however needs additional
 *   care when used in combination with automatic type deduction.
 * - array return types also result in a wrapper class that can be implicitly converted to a sequential
 *   container or a @p QAndroidJniObject representing the JNI array.
 *
 * Thie macro can only be placed in classes having the @c JNI_UNMANAGED_OBJECT or @c JNI_OBJECT macro.
 *
 * @param RetT The return type. Must either be a basic type or a type declared with @c JNI_TYPE
 * @param Name The name of the method. Must match the JNI method to be called exactly.
 * @param Args A list or argument types (can be empty). Must either be basic types or types declared
 *        with @c JNI_TYPE.
 */
#define JNI_STATIC_METHOD(RetT, Name, ...) \
static inline auto Name( JNI_PARAMS(__VA_ARGS__) ) { \
    using namespace KAndroidExtras; \
    return Internal::static_caller<RetT>::call(Jni::typeName<_jni_ThisType>(), "" #Name, Jni::signature<RetT(__VA_ARGS__)>() __VA_OPT__(,) JNI_ARGS(__VA_ARGS__)); \
}

/**
 * Wrap a JNI constructor call.
 * This will add a constructor named @p Name to the current class. Argument types are checked at compile time,
 * with the following inputs being accepted:
 * - basic types have to match exactly
 * - non-basic types can be either passed as @c QAndroidJniObject instance or with a type that has an
 *   conversion registered with @c JNI_DECLARE_CONVERTER.
 *
 * Thie macro can only be placed in classes having @c JNI_OBJECT macro.
 *
 * @param Name The name of the method. Must match the JNI method to be called exactly.
 * @param Args A list or argument types (can be empty). Must either be basic types or types declared
 *        with @c JNI_TYPE.
 */
#define JNI_CONSTRUCTOR(Name, ...) \
inline Name( JNI_PARAMS(__VA_ARGS__) ) { \
    using namespace KAndroidExtras; \
    setJniHandle(QAndroidJniObject(Jni::typeName<_jni_ThisType>(), Jni::signature<void(__VA_ARGS__)>() __VA_OPT__(,) JNI_ARGS(__VA_ARGS__))); \
}

}

#endif
