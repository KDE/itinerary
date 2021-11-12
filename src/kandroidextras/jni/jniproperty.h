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

namespace KAndroidExtras {

namespace Jni
{

template <typename T>
class Wrapper {
protected:
    typedef T JniBaseType;
};

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
}

#define JNI_CONSTANT(type, name) \
private: \
    struct name ## __NameHolder { static constexpr const char* jniName() { return "" #name; } }; \
public: \
    static inline const Jni::StaticProperty<type, JniBaseType, name ## __NameHolder, Jni::is_basic_type<type>::value> name;


#define JNI_PROPERTY(Class, type, name) \
private: \
    struct name ## __NameHolder { static constexpr const char* jniName() { return "" #name; } }; \
    struct name ## __OffsetHolder { static constexpr std::size_t offset() { return offsetof(Class, name); } }; \
public: \
    [[no_unique_address]] Jni::Property<type, Class, name ## __NameHolder, name ## __OffsetHolder, Jni::is_basic_type<type>::value> name;
}

#endif // KANDROIDEXTRAS_JNIPROPERTIES_H
