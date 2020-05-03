/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KANDROIDEXTRAS_JNIPROPERTIES_H
#define KANDROIDEXTRAS_JNIPROPERTIES_H

#include "jnisignature.h"
#include "jnitypes.h"

#include <QAndroidJniObject>

namespace KAndroidExtras {

namespace Internal {
template <typename T> struct is_basic_type : std::false_type {};
template <> struct is_basic_type<jint> : std::true_type {};
// TODO extend this to all basic JNI types
}

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
        return QAndroidJniObject::getStaticObjectField<jstring>(Jni::typeName<ClassType>(), Jni::typeName<NameHolder>());
    }
    inline operator QString() const
    {
        return QAndroidJniObject::getStaticObjectField<jstring>(Jni::typeName<ClassType>(), Jni::typeName<NameHolder>()).toString();
    }
};

template <typename PropType, typename ClassType, typename NameHolder>
struct StaticProperty<PropType, ClassType, NameHolder, true> {
    inline operator PropType() const
    {
        return QAndroidJniObject::getStaticField<PropType>(Jni::typeName<ClassType>(), Jni::typeName<NameHolder>());
    }
};

}

#define JNI_CONSTANT(type, name) \
private: \
    struct name ## __NameHolder { static constexpr const char* jniName() { return "" #name; } }; \
public: \
    static inline const Jni::StaticProperty<type, JniBaseType, name ## __NameHolder, Internal::is_basic_type<type>::value> name;
}

#endif // KANDROIDEXTRAS_JNIPROPERTIES_H
