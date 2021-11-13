/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNIPROPERTIES_H
#define KANDROIDEXTRAS_JNIPROPERTIES_H

#include "jnisignature.h"
#include "jnitypes.h"
#include "jnitypetraits.h"

#include <QAndroidJniObject>

#include <type_traits>

namespace KAndroidExtras {

namespace Jni
{

/** Annotates a class for holding JNI property wrappers.
 *  This class has to provide a jniName() method, which is usually achieved by inheriting from
 *  a type defined by the @c JNI_TYPE or @c JNI_NESTED_TYPE macros.
 *  For using non-static properties, this class also has to provide a @p handle() method returning
 *  a @c QAndroidJniObject representing the current instance.
 *
 *  @param Class the name of the class this is added to.
 */
#define JNI_OBJECT(Class) \
private: \
    typedef Class _jni_ThisType; \
    friend const char* KAndroidExtras::Jni::typeName<Class>();


/** @cond internal */

/** Wrapper for static properties. */
template <typename PropType, typename ClassType, typename NameHolder, bool BasicType> struct StaticProperty {};
template <typename PropType, typename ClassType, typename NameHolder>
struct StaticProperty<PropType, ClassType, NameHolder, false> {
    inline operator QAndroidJniObject() const
    {
        return QAndroidJniObject::getStaticObjectField(Jni::typeName<ClassType>(), Jni::typeName<NameHolder>(), Jni::signature<PropType>());
    }
};

template <typename ClassType, typename NameHolder>
struct StaticProperty<java::lang::String, ClassType, NameHolder, false> {
    inline operator QAndroidJniObject() const
    {
        return QAndroidJniObject::getStaticObjectField(Jni::typeName<ClassType>(), Jni::typeName<NameHolder>(), Jni::signature<java::lang::String>());
    }
    inline operator QString() const
    {
        return QAndroidJniObject::getStaticObjectField(Jni::typeName<ClassType>(), Jni::typeName<NameHolder>(), Jni::signature<java::lang::String>()).toString();
    }
};

template <typename PropType, typename ClassType, typename NameHolder>
struct StaticProperty<PropType, ClassType, NameHolder, true> {
    inline operator PropType() const
    {
        return QAndroidJniObject::getStaticField<PropType>(Jni::typeName<ClassType>(), Jni::typeName<NameHolder>());
    }
};

/** Shared code for non-static property wrappers. */
template <typename ClassType, typename OffsetHolder>
class PropertyBase {
protected:
    inline QAndroidJniObject& handle() {
        const auto owner = reinterpret_cast<ClassType*>(reinterpret_cast<char*>(this) - OffsetHolder::offset());
        return owner->m_handle;
    }
    inline const QAndroidJniObject& handle() const {
        const auto owner = reinterpret_cast<const ClassType*>(reinterpret_cast<const char*>(this) - OffsetHolder::offset());
        return owner->m_handle;
    }
};

/** Wrapper for non-static properties. */
template <typename PropType, typename ClassType, typename NameHolder, typename OffsetHolder, bool BasicType> struct Property {};
template <typename PropType, typename ClassType, typename NameHolder, typename OffsetHolder>
class Property<PropType, ClassType, NameHolder, OffsetHolder, false> : public PropertyBase<ClassType, OffsetHolder> {
public:
    inline operator QAndroidJniObject() const
    {
        return this->handle().getObjectField(Jni::typeName<NameHolder>(), Jni::signature<PropType>());
    }
    inline Property& operator=(const QAndroidJniObject &value)
    {
        this->handle().setField(Jni::typeName<NameHolder>(), Jni::signature<PropType>(), value.object());
        return *this;
    }
};

template <typename ClassType, typename NameHolder, typename OffsetHolder>
class Property<java::lang::String, ClassType, NameHolder, OffsetHolder, false> : public PropertyBase<ClassType, OffsetHolder> {
public:
    inline operator QAndroidJniObject() const
    {
        return this->handle().getObjectField(Jni::typeName<NameHolder>(), Jni::signature<java::lang::String>());
    }
    inline operator QString() const
    {
        return this->handle().getObjectField(Jni::typeName<NameHolder>(), Jni::signature<java::lang::String>()).toString();
    }
    inline Property& operator=(const QAndroidJniObject &value)
    {
        this->handle().setField(Jni::typeName<NameHolder>(), Jni::signature<java::lang::String>(), value.object());
        return *this;
    }
    inline Property& operator=(const QString &value)
    {
        this->handle().setField(Jni::typeName<NameHolder>(), Jni::signature<java::lang::String>(), QAndroidJniObject::fromString(value).object());
        return *this;
    }
};

template <typename PropType, typename ClassType, typename NameHolder, typename OffsetHolder>
class Property<PropType, ClassType, NameHolder, OffsetHolder, true> : public PropertyBase<ClassType, OffsetHolder> {
public:
    inline operator PropType() const
    {
        return this->handle().template getField<PropType>(Jni::typeName<NameHolder>());
    }
    inline Property& operator=(PropType value)
    {
        this->handle().setField(Jni::typeName<NameHolder>(), Jni::signature<PropType>(), value);
        return *this;
    }
};

/** @endcond */
}

/**
 * Wrap a static final property.
 * This will add a public static member named @p name to the current class. This member defines an
 * implicit conversion operator which will trigger the corresponding a JNI read operation.
 * Can only be placed in classes with a @c JNI_OBJECT.
 *
 * @note Make sure to access this member with a specific type, assigning to an @c auto variable will
 * copy the wrapper type, not read the property value.
 *
 * @param type The data type of the property.
 * @param name The name of the proeprty.
 */
#define JNI_CONSTANT(type, name) \
private: \
    struct _jni_ ## name ## __NameHolder { static constexpr const char* jniName() { return "" #name; } }; \
public: \
    static inline const Jni::StaticProperty<type, _jni_ThisType, _jni_ ## name ## __NameHolder, Jni::is_basic_type<type>::value> name;

/**
 * Wrap a member property.
 * This will add a public zero-size member named @p name to the current class. This member defines an
 * implicit conversion operator which will trigger the corresponding a JNI read operation, as well
 * as an overloaded assignment operator for the corresponding write operation.
 * Can only be placed in classes with a @c JNI_OBJECT.
 *
 * @note Make sure to access this member with a specific type, assigning to an @c auto variable will
 * copy the wrapper type, not read the property value.
 *
 * @param type The data type of the property.
 * @param name The name of the proeprty.
 */
#define JNI_PROPERTY(type, name) \
private: \
    struct _jni_ ## name ## __NameHolder { static constexpr const char* jniName() { return "" #name; } }; \
    struct _jni_ ## name ## __OffsetHolder { static constexpr std::size_t offset() { return offsetof(_jni_ThisType, name); } }; \
public: \
    [[no_unique_address]] Jni::Property<type, _jni_ThisType, _jni_ ## name ## __NameHolder, _jni_ ## name ## __OffsetHolder, Jni::is_basic_type<type>::value> name;
}

#endif // KANDROIDEXTRAS_JNIPROPERTIES_H
