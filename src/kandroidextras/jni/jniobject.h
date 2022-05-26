/*
    SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNIOBJECT_H
#define KANDROIDEXTRAS_JNIOBJECT_H

#include "jnitypetraits.h"

namespace KAndroidExtras {

namespace java { namespace lang { struct String; } }

namespace Jni {

template <typename T>
inline QAndroidJniObject handle(const T &wrapper)
{
    return wrapper.jniHandle();
}

/** Wrapper for JNI objects with a convertible C++ type.
 *  This provides implicit on-demand conversion to the C++ type, for types
 *  that don't have a manually defined wrapper.
 */
template <typename T>
class Object {
private:
    template <typename DummyT> class _dummy_t {};
public:
    inline Object(const QAndroidJniObject &v) : m_handle(v) {}

    // implicit conversion from a compatible C++ type
    inline Object(const std::conditional_t<std::is_same_v<typename Jni::converter<T>::type, void>,
                  _dummy_t<T>, typename Jni::converter<T>::type> &v)
        : m_handle(Jni::reverse_converter<T>::type::convert(v))
    {}

    // implicit conversion from a custom wrapper for the same type
    template <typename WrapperT, typename = std::enable_if_t<std::is_same_v<typename WrapperT::_jni_BaseType, T>, WrapperT>>
    inline Object(const WrapperT &w)
        : m_handle(Jni::handle(w))
    {}

    // special-case QString, as that often comes out of implicitly converted types,
    // and thus can't be consumed by another implicit conversion step
    template <typename StrT = QString, typename = std::enable_if_t<std::is_same_v<T, java::lang::String>, StrT>>
    inline Object(const QString &s)
        : m_handle(QAndroidJniObject::fromString(s))
    {}

    // special case for null values
    inline Object(std::nullptr_t)
        : m_handle((jobject)nullptr)
    {}

    inline operator QAndroidJniObject() const {
        return m_handle;
    }
    inline operator typename Jni::converter<T>::type() const {
        return Jni::converter<T>::convert(m_handle);
    }

    inline QAndroidJniObject jniHandle() const {
        return m_handle;
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
 *  Use this if the class only has static methods or constants. For methods
 *  and properties, you either need to use @c JNI_OBJECT instead, or make
 *  sure there is a @p jniHandle() method returning a @c QAndroidJniObject
 *  representing the current instance.
 *
 *  @param Class the name of the class this is added to.
 *  @param BaseType the Java type this class represents, defined by the
 *  @c JNI_TYPE or @c JNI_NESTED_TYPE macros.
 */
#define JNI_UNMANAGED_OBJECT(Class, BaseType) \
public: \
    typedef Class _jni_ThisType; \
    typedef BaseType _jni_BaseType; \
private: \
    static inline constexpr const char *jniName() { return KAndroidExtras::Jni::typeName<BaseType>(); } \
    friend constexpr const char* KAndroidExtras::Jni::typeName<Class>();

/** Annotates a class for holding JNI method or property wrappers.
 *
 *  @param Class the name of the class this is added to.
 *  @param BaseType the Java type this class represents, defined by the
 *  @c JNI_TYPE or @c JNI_NESTED_TYPE macros.
 */
#define JNI_OBJECT(Class, BaseType) \
    JNI_UNMANAGED_OBJECT(Class, BaseType) \
private: \
    QAndroidJniObject _m_jni_handle; \
    inline QAndroidJniObject jniHandle() const { return _m_jni_handle; } \
    inline void setJniHandle(const QAndroidJniObject &h) { _m_jni_handle = h; } \
    friend QAndroidJniObject KAndroidExtras::Jni::handle<Class>(const Class&); \
public: \
    explicit Class(const QAndroidJniObject &h) : _m_jni_handle(h) {}
}
}

#endif
