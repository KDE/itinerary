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

/** Type trait to check whether @tparam T is a basic JNI type or an object type.
 *  Those two typically need different handling both with JNI API and with QAndroidJniObject API.
 */
template <typename T> struct is_basic_type : std::false_type {};
template <> struct is_basic_type<bool> : std::true_type {};
template <> struct is_basic_type<jbyte> : std::true_type {};
template <> struct is_basic_type<jchar> : std::true_type {};
template <> struct is_basic_type<jshort> : std::true_type {};
template <> struct is_basic_type<jint> : std::true_type {};
template <> struct is_basic_type<jlong> : std::true_type {};
template <> struct is_basic_type<jfloat> : std::true_type {};
template <> struct is_basic_type<jdouble> : std::true_type {};


template <typename T> struct converter {
    typedef void type;
};

template <typename T> struct reverse_converter {
    typedef converter<typename converter<T>::type> type;
};

}

}

#endif
