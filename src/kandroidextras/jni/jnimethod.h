/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNIMETHOD_H
#define KANDROIDEXTRAS_JNIMETHOD_H

#include "jnitypetraits.h"

#include <QAndroidJniObject>

namespace KAndroidExtras {
///@cond internal
namespace Internal {

    // argument compatibility checking
    template <typename SigT, typename ArgT> struct is_argument_compatible
    {
        static inline constexpr bool value =
            !std::is_same<SigT, void>::value && !std::is_same<ArgT, void>::value &&
            (std::is_same<SigT, ArgT>::value ||
            (!Jni::is_basic_type<SigT>::value && std::is_same<ArgT, QAndroidJniObject>::value) ||
            (!Jni::is_basic_type<SigT>::value && std::is_same<ArgT, typename Jni::converter<SigT>::type>::value));
    };

    template <typename ...Sig> struct is_call_compatible
    {
        template <typename ...Args> struct with {
            template<typename... Values>
            inline static constexpr bool all(Values... values) { return (... && values); }
            static inline constexpr bool value = all(is_argument_compatible<Sig, Args>::value...);
        };
    };

    // argument type conversion
    template <typename ArgT, bool is_basic, bool is_convertible> struct call_argument {};
    template <typename ArgT> struct call_argument<ArgT, true, false>
    {
        inline constexpr ArgT operator()(ArgT value) const
        {
            return value;
        }
    };
    template <typename ArgT> struct call_argument<ArgT, false, true>
    {
        inline constexpr jobject operator()(const ArgT &value) const
        {
            return Jni::converter<ArgT>::convert(value).object();
        }
    };
    template <> struct call_argument<QAndroidJniObject, false, false>
    {
        inline jobject operator()(const QAndroidJniObject &value) const
        {
            return value.object();
        }
    };

    template <typename ArgT> constexpr call_argument<ArgT, Jni::is_basic_type<ArgT>::value, !std::is_same_v<typename Jni::converter<ArgT>::type, void>> toCallArgument = {};

    // return type conversion
    template <typename RetT>
    struct call_return {
        static inline constexpr bool is_basic = Jni::is_basic_type<RetT>::value;
        static inline constexpr bool is_convertible = !std::is_same_v<typename Jni::converter<RetT>::type, void>;

        typedef std::conditional_t<is_basic || !is_convertible, RetT, typename Jni::converter<RetT>::type> CallReturnT;
        typedef std::conditional_t<is_basic, RetT, QAndroidJniObject> JniReturnT;

        static inline constexpr CallReturnT toReturnValue(JniReturnT value)
        {
            if constexpr (is_convertible) {
                return Jni::converter<RetT>::convert(value);
            } else {
                return value;
            }
        }
    };
    template <>
    struct call_return<void> {
        typedef void CallReturnT;
    };

    // call wrapper
    template <typename RetT, typename ...Sig>
    struct invoker {
        template <typename ...Args>
        static typename Internal::call_return<RetT>::CallReturnT call(QAndroidJniObject handle, const char *name, const char *signature, Args... args)
        {
            static_assert(is_call_compatible<Sig...>::template with<Args...>::value, "incompatible call arguments");
            if constexpr (Jni::is_basic_type<RetT>::value) {
                return handle.callMethod<RetT>(name, signature, toCallArgument<Args>(args)...);
            } else {
                return Internal::call_return<RetT>::toReturnValue(handle.callObjectMethod(name, signature, toCallArgument<Args>(args)...));
            }
            return {};
        }
    };

    template <typename ...Sig>
    struct invoker<void, Sig...> {
        template <typename ...Args>
        static void call(QAndroidJniObject handle, const char *name, const char *signature, Args... args)
        {
            static_assert(is_call_compatible<Sig...>::template with<Args...>::value, "incompatible call arguments");
            handle.callMethod<void>(name, signature, toCallArgument<Args>(std::forward<Args>(args))...);
        }
    };

    template <typename RetT>
    struct invoker<RetT> {
        static typename Internal::call_return<RetT>::CallReturnT call(QAndroidJniObject handle, const char *name, const char *signature)
        {
            if constexpr (Jni::is_basic_type<RetT>::value) {
                return handle.callMethod<RetT>(name, signature);
            } else {
                return Internal::call_return<RetT>::toReturnValue(handle.callObjectMethod(name, signature));
            }
        }
    };

    template <>
    struct invoker<void> {
        static void call(QAndroidJniObject handle, const char *name, const char *signature)
        {
            handle.callMethod<void>(name, signature);
        }
    };
}
///@endcond

/**
 * Wrap a JNI method call.
 * This will add a method named @p name to the current class. Argument types are checked at compile time,
 * with the following inputs being accepted:
 * - basic types have to match exactly
 * - non-basic types can be either passed as @c QAndroidJniObject instance or with a type that has an
 *   conversion registered with @c JNI_DECLARE_CONVERTER.
 * The return type is handled similarly, if a conversion to a native type is available that is returned,
 * @c QAndroidJniObject otherwise.
 * Can only be placed in classes with a @c JNI_OBJECT.
 *
 * @param RetT The return type. Must either be a basic type or a type declared with @c JNI_TYPE
 * @param Name The name of the method. Must match the JNI method to be called exactly.
 * @param Args A list or argument types (can be empty). Must either be basic types or types declared
 *        with @c JNI_TYPE.
 */
#define JNI_METHOD(RetT, Name, ...) \
template <typename ...Args> \
inline Internal::call_return<RetT>::CallReturnT Name(Args... args) const { \
    return Internal::invoker<RetT, ## __VA_ARGS__>::call(handle(), "" #Name, Jni::signature<RetT(__VA_ARGS__)>(), std::forward<Args>(args)...); \
}

}

#endif
