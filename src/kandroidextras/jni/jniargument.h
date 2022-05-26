/*
    SPDX-FileCopyrightText: 2021-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNIARGUMENTVALUE_H
#define KANDROIDEXTRAS_JNIARGUMENTVALUE_H

#include "jniobject.h"
#include "jnitypetraits.h"

#include <QAndroidJniObject>

namespace KAndroidExtras {
namespace Jni {
template <typename T> class Array;
}

///@cond internal
namespace Internal {
    /** Call argument wrapper. */
    template <typename T, typename = std::void_t<>>
    struct argument {
        typedef std::conditional_t<Jni::is_basic_type<T>::value, T, const Jni::Object<T>&> type;
        static inline constexpr auto toCallArgument(type value)
        {
            if constexpr (Jni::is_basic_type<T>::value) {
                return value;
            } else {
                return value.jniHandle().object();
            }
        }
    };
    template <typename T>
    struct argument<T, std::void_t<typename T::_jni_ThisType>> {
        typedef const T &type;
        static inline auto toCallArgument(const T &value)
        {
            return Jni::handle(value).object();
        }
    };
    template <typename T>
    struct argument<Jni::Array<T>> {
        typedef const Jni::Array<T>& type;
        static inline auto toCallArgument(const Jni::Array<T> &value)
        {
            return value.jniHandle().object();
        }
    };
}
///@endcond
}

#endif

