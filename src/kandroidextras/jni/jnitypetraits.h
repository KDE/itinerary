/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNITYPETRAITS_H
#define KANDROIDEXTRAS_JNITYPETRAITS_H

#include "jnitypes.h"

#include <QAndroidJniObject>

#include <type_traits>

namespace KAndroidExtras {

namespace Jni {

template <typename T> class Array;

/** Type trait to check whether @tparam T is a basic JNI type or an object type.
 *  Those two typically need different handling both with JNI API and with QAndroidJniObject API.
 */
template <typename T> struct is_basic_type : std::false_type {};
template <> struct is_basic_type<bool> : std::true_type {};
template <> struct is_basic_type<jboolean> : std::true_type {};
template <> struct is_basic_type<jbyte> : std::true_type {};
template <> struct is_basic_type<jchar> : std::true_type {};
template <> struct is_basic_type<jshort> : std::true_type {};
template <> struct is_basic_type<jint> : std::true_type {};
template <> struct is_basic_type<jlong> : std::true_type {};
template <> struct is_basic_type<jfloat> : std::true_type {};
template <> struct is_basic_type<jdouble> : std::true_type {};
template <> struct is_basic_type<void> : std::true_type {};

/** Type conversion trait, see @c JNI_DECLARE_CONVERTER. */
template <typename T> struct converter {
    typedef void type;
};

template <typename T> struct reverse_converter {
    typedef converter<typename converter<T>::type> type;
};

/** Type trait for checking whether @tparam T is a JNI array type. */
template <typename T> struct is_array : std::false_type {};
template <typename T> struct is_array<Array<T>> : std::true_type {};

/** Type trais for checking whether @tparam T is a JNI object wrapper type. */
template <typename T, typename = std::void_t<>> struct is_object_wrapper : std::false_type {};
template <typename T> struct is_object_wrapper<T, std::void_t<typename T::_jni_ThisType>> : std::true_type {};

/** Type trais for checking whether @tparam T is needs the generic JNI object wrapper (Jni::Object). */
template <typename T> struct is_generic_wrapper : std::conditional_t<
    !is_basic_type<T>::value && !is_array<T>::value && !is_object_wrapper<T>::value, std::true_type, std::false_type>
    {};

}

/**
 * Declare a JNI type to be convertible to a native type.
 * @param JniType A type declared with @p JNI_TYPE.
 * @param NativeType A C++ type @p JniType can be converted to/from.
 * @param FromJniFn Code converting a @c QAndroidJniObject @c value to @p NativeType.
 * @param ToJniCode converting a @p NativeType @c value to a QAndroidJniObject.
 */
#define JNI_DECLARE_CONVERTER(JniType, NativeType, FromJniFn, ToJniFn) \
namespace Jni { \
template <> struct converter<NativeType> { \
    typedef JniType type; \
    static inline QAndroidJniObject convert(const NativeType &value) { return (ToJniFn); } \
}; \
template <> struct converter<JniType> { \
    typedef NativeType type; \
    static inline NativeType convert(const QAndroidJniObject &value) { return (FromJniFn); } \
}; \
}

}

#endif
