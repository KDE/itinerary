/*
    SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNICOMMON_H
#define KANDROIDEXTRAS_JNICOMMON_H

#include "jnitypetraits.h"

namespace KAndroidExtras {

namespace java { namespace lang { struct String; } }

namespace Jni {

/** Wrapper for JNI objects with a convertible C++ type.
 *  This provides implicit on-demand conversion to the C++ type, for types
 *  that don't have a manually defined wrapper.
 */
template <typename T>
class Object {
public:
    explicit inline Object(const QAndroidJniObject &v) : m_handle(v) {}
    inline operator QAndroidJniObject() const {
        return m_handle;
    }
    inline operator typename Jni::converter<T>::type() const {
        return Jni::converter<T>::convert(m_handle);
    }

    // forward basic QAndroidJniObject API
    inline bool isValid() const {
        return m_handle.isValid();
    }
    template <typename StrT = QString, typename = std::enable_if_t<std::is_same_v<T, java::lang::String>, StrT>>
    inline QString toString() const {
        return m_handle.toString();
    }
private:
    QAndroidJniObject m_handle;
};

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
}

#endif
