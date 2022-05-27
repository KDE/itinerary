/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNIARRAY_H
#define KANDROIDEXTRAS_JNIARRAY_H

#include "jniargument.h"
#include "jnireturnvalue.h"
#include "jnitypetraits.h"

#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>

namespace KAndroidExtras {

///@cond internal
namespace Internal {

/** Basic type array type traits. */
template <typename T> struct array_trait {
    typedef jobjectArray type;
};

#define MAKE_ARRAY_TRAIT(base_type, type_name) \
template <> struct array_trait<base_type> { \
    typedef base_type ## Array type; \
    static inline type newArray(JNIEnv *env, jsize size) { return env->New ## type_name ## Array(size); } \
    static inline base_type* getArrayElements(JNIEnv *env, type array, jboolean *isCopy) { return env->Get ## type_name ## ArrayElements(array, isCopy); } \
    static inline void releaseArrayElements(JNIEnv *env, type array, base_type *data, jint mode) { return env->Release ## type_name ## ArrayElements(array, data, mode); } \
    static inline void setArrayRegion(JNIEnv *env, type array, jsize start, jsize length, const base_type *data) { env->Set ## type_name ## ArrayRegion(array, start, length, data); } \
};

MAKE_ARRAY_TRAIT(jboolean, Boolean)
MAKE_ARRAY_TRAIT(jbyte, Byte)
MAKE_ARRAY_TRAIT(jchar, Char)
MAKE_ARRAY_TRAIT(jshort, Short)
MAKE_ARRAY_TRAIT(jint, Int)
MAKE_ARRAY_TRAIT(jlong, Long)
MAKE_ARRAY_TRAIT(jfloat, Float)
MAKE_ARRAY_TRAIT(jdouble, Double)

#undef MAKE_ARRAY_TRAIT

/** Meta function for retrieving a JNI array .*/
template <typename Container, typename Value, bool is_basic> struct FromArray {};

template <typename Container>
struct FromArray<Container, QAndroidJniObject, false>
{
    inline auto operator()(const QAndroidJniObject &array) const
    {
        if (!array.isValid()) {
            return Container{};
        }
        const auto a = static_cast<jobjectArray>(array.object());
        QAndroidJniEnvironment env;
        const auto size = env->GetArrayLength(a);
        Container r;
        r.reserve(size);
        for (auto i = 0; i < size; ++i) {
            r.push_back(QAndroidJniObject::fromLocalRef(env->GetObjectArrayElement(a, i)));
        }
        return r;
    }
};

template <typename Container, typename Value>
struct FromArray<Container, Value, false>
{
    inline auto operator()(const QAndroidJniObject &array) const
    {
        if (!array.isValid()) {
            return Container{};
        }
        const auto a = static_cast<jobjectArray>(array.object());
        QAndroidJniEnvironment env;
        const auto size = env->GetArrayLength(a);
        Container r;
        r.reserve(size);
        for (auto i = 0; i < size; ++i) {
            r.push_back(Jni::reverse_converter<Value>::type::convert(QAndroidJniObject::fromLocalRef(env->GetObjectArrayElement(a, i))));
        }
        return r;
    }
};

// specializations for basic types
template <typename Container, typename Value>
struct FromArray<Container, Value, true>
{
    typedef array_trait<Value> _t;
    inline auto operator()(const QAndroidJniObject &array) const
    {
        if (!array.isValid()) {
            return Container{};
        }

        const auto a = static_cast<typename _t::type>(array.object());
        QAndroidJniEnvironment env;
        const auto size = env->GetArrayLength(a);
        Container r;
        r.reserve(size);

        auto data = _t::getArrayElements(env, a, nullptr);
        std::copy(data, data + size, std::back_inserter(r));
        _t::releaseArrayElements(env, a, data, JNI_ABORT);

        return r;
    }
};

// array wrapper, common base for basic and non-basic types
template <typename T>
class ArrayImplBase {
public:
    typedef T value_type;
    typedef jsize size_type;
    typedef jsize difference_type;

    ArrayImplBase() = default;
    inline ArrayImplBase(const QAndroidJniObject &array) : m_array(array) {}
    ArrayImplBase(const ArrayImplBase &) = default;
    ArrayImplBase(ArrayImplBase &&) = default;

    inline size_type size() const
    {
        if (!m_array.isValid()) {
            return 0;
        }
        const auto a = static_cast<typename _t::type>(m_array.object());
        QAndroidJniEnvironment env;
        return env->GetArrayLength(a);
    }

    inline operator QAndroidJniObject() const {
        return m_array;
    }
    inline QAndroidJniObject jniHandle() const { return m_array; }

protected:
    typedef array_trait<T> _t;

    typename _t::type handle() const { return static_cast<typename _t::type>(m_array.object()); }

    QAndroidJniObject m_array;
};

template <typename T, bool is_basic>
class ArrayImpl {};

// array wrapper for primitive types
template <typename T>
class ArrayImpl<T, true> : public ArrayImplBase<T>
{
public:
    inline ArrayImpl(const QAndroidJniObject &array)
        : ArrayImplBase<T>(array)
    {
        // ### do this on demand?
        getArrayElements();
    }

    /** Create a new array with @p size elements. */
    inline explicit ArrayImpl(jsize size)
    {
        QAndroidJniEnvironment env;
        ArrayImplBase<T>::m_array = QAndroidJniObject::fromLocalRef(ArrayImplBase<T>::_t::newArray(env, size));
        getArrayElements();
    }

    ArrayImpl() = default;
    ArrayImpl(const ArrayImpl&) = delete; // ### ref count m_data and allow copying?
    ArrayImpl(ArrayImpl&&) = default;
    ~ArrayImpl()
    {
        QAndroidJniEnvironment env;
        ArrayImplBase<T>::_t::releaseArrayElements(env, this->handle(), m_data, JNI_ABORT);
    }

    T operator[](jsize index) const
    {
        return m_data[index];
    }

    T* begin() const { return m_data; }
    T* end() const { return m_data + ArrayImplBase<T>::size(); }

private:
    inline void getArrayElements()
    {
        if (!ArrayImplBase<T>::m_array.isValid()) {
            return;
        }
        QAndroidJniEnvironment env;
        m_data = ArrayImplBase<T>::_t::getArrayElements(env, this->handle(), nullptr);
    }

    T *m_data = nullptr;
};

// array wrapper for non-primitive types
template <typename T>
class ArrayImpl<T, false> : public ArrayImplBase<T>
{
public:
    using ArrayImplBase<T>::ArrayImplBase;

    /** Create a new array with @p size elements initialized with @p value. */
    explicit inline ArrayImpl(jsize size, typename Internal::argument<T>::type value)
    {
        QAndroidJniEnvironment env;
        auto clazz = env.findClass(Jni::typeName<T>());
        ArrayImplBase<T>::m_array = QAndroidJniObject::fromLocalRef(env->NewObjectArray(size, clazz, Internal::argument<T>::toCallArgument(value)));
    }

    /** Create a new array with @p size null elements. */
    explicit inline ArrayImpl(jsize size, std::nullptr_t = nullptr)
    {
        QAndroidJniEnvironment env;
        auto clazz = env.findClass(Jni::typeName<T>());
        ArrayImplBase<T>::m_array = QAndroidJniObject::fromLocalRef(env->NewObjectArray(size, clazz, nullptr));
    }

    ArrayImpl() = default;
    ArrayImpl(const ArrayImpl&) = default;
    ArrayImpl(ArrayImpl&&) = default;

    auto operator[](jsize index) const
    {
        QAndroidJniEnvironment env;
        return Internal::return_wrapper<T>::toReturnValue(QAndroidJniObject::fromLocalRef(env->GetObjectArrayElement(this->handle(), index)));
    }

    class ref
    {
    public:
        inline operator auto()
        {
            QAndroidJniEnvironment env;
            return Internal::return_wrapper<T>::toReturnValue(QAndroidJniObject::fromLocalRef(env->GetObjectArrayElement(c.handle(), index)));
        }
        inline ref& operator=(typename Internal::argument<T>::type v)
        {
            QAndroidJniEnvironment env;
            env->SetObjectArrayElement(c.handle(), index, Internal::argument<T>::toCallArgument(v));
            return *this;
        }
    private:
        ArrayImpl<T, false> &c;
        jsize index;

        friend class ArrayImpl<T, false>;
        inline ref(jsize _i, ArrayImpl<T, false> &_c) : c(_c), index(_i) {}
    };
    ref operator[](jsize index)
    {
        return ref(index, *this);
    }

    class const_iterator
    {
        const ArrayImpl<T, false> &c;
        jsize i = 0;
    public:
        typedef jsize difference_type;
        typedef T value_type;
        typedef T& reference;
        typedef std::random_access_iterator_tag iterator_category;
        typedef T* pointer;

        const_iterator(const ArrayImpl<T, false> &_c, jsize _i) : c(_c), i(_i) {}

        difference_type operator-(const_iterator other) const { return i - other.i; }

        const_iterator& operator++() { ++i; return *this; }
        const_iterator operator++(int) { return const_iterator(c, i++); }

        bool operator==(const_iterator other) const { return i == other.i; }
        bool operator!=(const_iterator other) const { return i != other.i; }

        auto operator*() const {
            return c[i];
        }
    };

    const_iterator begin() const { return const_iterator(*this, 0); }
    const_iterator end() const { return const_iterator(*this, ArrayImplBase<T>::size()); }

};

}
///@endcond

namespace Jni {

/** Convert a JNI array to a C++ container.
    *  Container value types can be any of
    *  - QAndroidJniObject
    *  - a basic JNI type
    *  - a type with a conversion defined with @c JNI_DECLARE_CONVERTER
    */
template <typename Container> constexpr __attribute__((__unused__)) Internal::FromArray<Container, typename Container::value_type, Jni::is_basic_type<typename Container::value_type>::value> fromArray = {};

/** Container-like wrapper for JNI arrays. */
template <typename T>
class Array : public Internal::ArrayImpl<T, Jni::is_basic_type<T>::value> {
public:
    using Internal::ArrayImpl<T, Jni::is_basic_type<T>::value>::ArrayImpl;
    template <typename Container>
    inline operator Container() const {
        // ### should this be re-implemented in terms of Jni::Array API rather than direct JNI access?
        return Jni::fromArray<Container>(this->m_array);
    }
};
}

}

#endif
