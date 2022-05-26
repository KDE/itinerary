/*
    SPDX-FileCopyrightText: 2021-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNIRETURNVALUE_H
#define KANDROIDEXTRAS_JNIRETURNVALUE_H

#include "jniobject.h"
#include "jnitypetraits.h"

#include <QAndroidJniObject>

namespace KAndroidExtras {
namespace Jni {
template <typename T> class Array;
}

///@cond internal
namespace Internal {
    /** Return value wrapping helper. */
    template <typename RetT, typename = std::void_t<>>
    class return_wrapper {
        static inline constexpr bool is_basic = Jni::is_basic_type<RetT>::value;
        static inline constexpr bool is_convertible = !std::is_same_v<typename Jni::converter<RetT>::type, void>;

        typedef std::conditional_t<is_basic, RetT, QAndroidJniObject> JniReturnT;

    public:
        static inline constexpr auto toReturnValue(JniReturnT value)
        {
            if constexpr (is_convertible) {
                return Jni::Object<RetT>(value);
            } else {
                return value;
            }
        }
    };
    template <typename RetT>
    class return_wrapper<RetT, std::void_t<typename RetT::_jni_ThisType>> {
    public:
        static inline auto toReturnValue(const QAndroidJniObject &value)
        {
            return Jni::fromHandle<RetT>(value);
        }
    };
    template <typename RetT>
    class return_wrapper<Jni::Array<RetT>> {
    public:
        static inline auto toReturnValue(const QAndroidJniObject &value)
        {
            return Jni::Array<RetT>(value);
        }
    };
    template <>
    struct return_wrapper<void> {};
}
///@endcond
}

#endif
