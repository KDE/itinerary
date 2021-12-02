/*
    SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNICOMMON_H
#define KANDROIDEXTRAS_JNICOMMON_H

namespace KAndroidExtras {

namespace Jni {

/** Wrapper type for array return values (which we cannot specify using [] syntax). */
template <typename T> struct Array {};

}

/** Annotates a class for holding JNI method or property wrappers.
 *
 *  For using non-static methods or properties, this class also has to provide a
 *  @p handle() method returning a @c QAndroidJniObject representing the current
 *  instance.
 *
 *  @param Class the name of the class this is added to.
 *  @param BaseType the Java type this class represents, defined by the
 *  @c JNI_TYPE or @c JNI_NESTED_TYPE macros.
 */
#define JNI_OBJECT(Class, BaseType) \
private: \
    typedef Class _jni_ThisType; \
    static inline constexpr const char *jniName() { return KAndroidExtras::Jni::typeName<BaseType>(); } \
    friend constexpr const char* KAndroidExtras::Jni::typeName<Class>();

}

#endif
