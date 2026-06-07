/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KJNIEXTRAS_JNIPROPERTIES_H
#define KJNIEXTRAS_JNIPROPERTIES_H

#include "kjniobject.h"

///@cond internal
namespace KJniExtras::Detail
{

/** Wrapper for static properties. */
template<typename PropType, typename ClassType, typename NameHolder>
struct StaticProperty {
    operator auto() const
    {
        return ClassType::template getStaticField<PropType>(NameHolder::jniName());
    }
};

/** Wrapper for non-static properties. */
template<typename PropType, typename ClassType, typename NameHolder, typename OffsetHolder>
class Property
{
private:
    ClassType* getThis()
    {
        return reinterpret_cast<ClassType *>(reinterpret_cast<char *>(this) - OffsetHolder::offset());
    }
    const ClassType* getThis() const
    {
        return reinterpret_cast<const ClassType *>(reinterpret_cast<const char *>(this) - OffsetHolder::offset());
    }
public:
    operator PropType() const
    {
        return getThis()->template getField<PropType>(NameHolder::jniName());
    }
    Property &operator=(std::conditional_t<QtJniTypes::isPrimitiveType<PropType>(), PropType, const PropType&> value)
    {
        getThis()->setField(NameHolder::jniName(), value);
        return *this;
    }
};

}
///@endcond

/**
 * Wrap a static final property.
 * This will add a public static member named @p name to the current class. This member defines an
 * implicit conversion operator which will trigger the corresponding a JNI read operation.
 * Can only be placed in classes with a @c KJNI_OBJECT.
 *
 * @note Make sure to access this member with a specific type, assigning to an @c auto variable will
 * copy the wrapper type, not read the property value.
 *
 * @param type The data type of the property.
 * @param name The name of the property.
 */
#define KJNI_CONSTANT(type, name) \
private: \
    struct _kjni_##name##__NameHolder { \
        static constexpr const char *jniName() { return "" #name; } \
    }; \
public: \
    static inline const KJniExtras::Detail::StaticProperty<type, _kjni_ThisType, _kjni_##name##__NameHolder> name;

/**
 * Wrap a member property.
 * This will add a public zero-size member named @p name to the current class. This member defines an
 * implicit conversion operator which will trigger the corresponding a JNI read operation, as well
 * as an overloaded assignment operator for the corresponding write operation.
 * Can only be placed in classes with a @c KJNI_OBJECT.
 *
 * @note Make sure to access this member with a specific type, assigning to an @c auto variable will
 * copy the wrapper type, not read the property value.
 *
 * @param type The data type of the property.
 * @param name The name of the property.
 */
#define KJNI_PROPERTY(type, name) \
private: \
    struct _kjni_##name##__NameHolder { \
        static constexpr const char *jniName() { return "" #name; } \
    }; \
    struct _kjni_##name##__OffsetHolder { \
        static constexpr std::size_t offset() \
        { \
            QT_WARNING_PUSH QT_WARNING_DISABLE_INVALID_OFFSETOF return offsetof(_kjni_ThisType, name); \
            QT_WARNING_POP \
        } \
    }; \
public: \
    [[no_unique_address]] KJniExtras::Detail::Property<type, _kjni_ThisType, _kjni_##name##__NameHolder, _kjni_##name##__OffsetHolder> name;

#endif
