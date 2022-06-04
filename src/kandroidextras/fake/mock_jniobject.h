/*
    SPDX-FileCopyrightText: 2020-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_MOCK_JNIOBJECT_H
#define KANDROIDEXTRAS_MOCK_JNIOBJECT_H

#include "kandroidextras_export.h"
#include "jni.h"

#include <kandroidextras/jnisignature.h>
#include <kandroidextras/jnitypes.h>

#include <QExplicitlySharedDataPointer>
#include <QStringList>
#include <QVariant>

class MockJniObjectBasePrivate;

namespace KAndroidExtras {
namespace Internal {
template <typename T>
constexpr inline const char* argTypeToString()
{
    return KAndroidExtras::Jni::signature<T>();
}
template <>
constexpr inline const char* argTypeToString<jobject>()
{
    return "o";
}

class KANDROIDEXTRAS_EXPORT MockJniObjectBase {
public:
    MockJniObjectBase();
    MockJniObjectBase(const MockJniObjectBase&);
    ~MockJniObjectBase();
    MockJniObjectBase& operator=(const MockJniObjectBase&);

    MockJniObjectBase(const char *className);

    inline MockJniObjectBase(const char *className, const char *signature)
        : MockJniObjectBase()
    {
        addToProtocol(QLatin1String("ctor: ") + QLatin1String(className) + QLatin1Char(' ') + QLatin1String(signature));
    }
    template <typename ...Args>
    inline MockJniObjectBase(const char *className, const char *signature, Args...)
        : MockJniObjectBase()
    {
        addToProtocol(QLatin1String("ctor: ") + QLatin1String(className) + QLatin1Char(' ') + QLatin1String(signature)
            + QLatin1String(" (") + (...+QLatin1String(KAndroidExtras::Internal::argTypeToString<Args>())) + QLatin1Char(')'));
    }

    MockJniObjectBase(jobject object);

    bool isValid() const { return true; }

    static QStringList m_staticProtocol;

    QStringList protocol() const;
    void addToProtocol(const QString &line) const;

protected:
    QVariant property(const QByteArray &name) const;
    void setProperty(const QByteArray &name, const QVariant &value);
    QVariant value() const;
    void setValue(const QVariant &value);
    void setData(jobject object);

    void setProtocol(const QStringList &protocol);
    QExplicitlySharedDataPointer<MockJniObjectBasePrivate> d;
};


template <typename JniObjectT>
class MockJniObject : public MockJniObjectBase {
public:
    using MockJniObjectBase::MockJniObjectBase;
    static inline JniObjectT fromString(const QString &s)
    {
        JniObjectT o;
        o.setValue(s);
        return o;
    }
    static inline JniObjectT fromLocalRef(jobject o)
    {
        JniObjectT obj;
        obj.addToProtocol(QLatin1String("ctor: ") + QString::number((qulonglong)o));
        return obj;
    }
    inline QString toString() const
    {
        return value().type() == QVariant::String ? value().toString() : protocol().join(QLatin1Char('\n'));
    }

    inline jobject object() const { return d.data(); }
    template <typename T>
    inline T object() const { return {}; }

    template <typename T, typename ...Args>
    inline T callMethod(const char *methodName, const char *signature, Args...) const
    {
        const QString s = QLatin1String("callMethod: ") + QLatin1String(methodName) + QLatin1Char(' ') + QLatin1String(signature) + QLatin1String(" (")
            + (...+QLatin1String(KAndroidExtras::Internal::argTypeToString<Args>())) + QLatin1Char(')');
        addToProtocol(s);
        if constexpr (!std::is_same_v<T, void>) {
            return {};
        }
    }
    template <typename T>
    inline T callMethod(const char *methodName, const char *signature) const
    {
        const QString s = QLatin1String("callMethod: ") + QLatin1String(methodName) + QLatin1Char(' ') + QLatin1String(signature) + QLatin1String(" ()");
        addToProtocol(s);
        if constexpr (!std::is_same_v<T, void>) {
            return {};
        }
    }

    template <typename ...Args>
    inline JniObjectT callObjectMethod(const char *methodName, const char *signature, Args...) const
    {
        const QString s = QLatin1String("callObjectMethod: ") + QLatin1String(methodName) + QLatin1Char(' ') + QLatin1String(signature) + QLatin1String(" (")
            + (...+QLatin1String(KAndroidExtras::Internal::argTypeToString<Args>())) + QLatin1Char(')');
        addToProtocol(s);

        JniObjectT obj;
        obj.setProtocol(protocol());
        return obj;
    }
    inline JniObjectT callObjectMethod(const char *methodName, const char *signature) const
    {
        addToProtocol(QLatin1String("callObjectMethod: ") + QLatin1String(methodName) + QLatin1Char(' ') + QLatin1String(signature) + QLatin1String(" ()"));

        JniObjectT obj;
        obj.setProtocol(protocol());
        return obj;
    }

    template <typename T>
    inline T getField(const char *fieldName) const
    {
        addToProtocol(QLatin1String("getField: ") + QLatin1String(fieldName) + QLatin1Char(' ') + QLatin1String(KAndroidExtras::Jni::signature<T>()));
        return property(fieldName).template value<T>();
    }

    inline JniObjectT getObjectField(const char *fieldName, const char *signature) const
    {
        addToProtocol(QLatin1String("getObjectField: ") + QLatin1String(fieldName) + QLatin1Char(' ') + QLatin1String(signature));
        return property(fieldName).template value<JniObjectT>();
    }

    template <typename T>
    inline void setField(const char *fieldName, const char *signature, T value)
    {
        Q_UNUSED(value);
        addToProtocol(QLatin1String("setField: ") + QLatin1String(fieldName) + QLatin1Char(' ') + QLatin1String(signature));
        if constexpr (std::is_same_v<jobject, T>) {
            setObjectProperty(fieldName, value);
        } else {
            setProperty(fieldName, QVariant::fromValue(value));
        }
    }
    template <typename T>
    inline void setField(const char *fieldName, T value)
    {
        setField(fieldName, KAndroidExtras::Jni::signature<T>(), value);
    }

    template <typename T, typename ...Args>
    static inline T callStaticMethod(const char *className, const char *methodName, const char *signature, Args...)
    {
        const QString s = QLatin1String("callStaticMethod: ") + QLatin1String(className) + QLatin1Char(' ')
            + QLatin1String(methodName) + QLatin1Char(' ') + QLatin1String(signature) + QLatin1String(" (")
            + (...+QLatin1String(KAndroidExtras::Internal::argTypeToString<Args>())) + QLatin1Char(')');
        m_staticProtocol.push_back(s);
        if constexpr (!std::is_same_v<T, void>) {
            return {};
        }
    }
    template <typename T>
    static inline T callStaticMethod(const char *className, const char *methodName, const char *signature)
    {
        const QString s = QLatin1String("callStaticMethod: ") + QLatin1String(className) + QLatin1Char(' ')
            + QLatin1String(methodName) + QLatin1Char(' ') + QLatin1String(signature) + QLatin1String(" ()");
        m_staticProtocol.push_back(s);
        if constexpr (!std::is_same_v<T, void>) {
            return {};
        }
    }

    template <typename ...Args>
    static inline JniObjectT callStaticObjectMethod(const char *className, const char *methodName, const char *signature, Args...)
    {
        const QString s = QLatin1String("callStaticObjectMethod: ") + QLatin1String(className) + QLatin1Char(' ')
            + QLatin1String(methodName) + QLatin1Char(' ') + QLatin1String(signature) + QLatin1String(" (")
            + (...+QLatin1String(KAndroidExtras::Internal::argTypeToString<Args>())) + QLatin1Char(')');
        JniObjectT obj;
        obj.addToProtocol(s);
        return obj;
    }

    static inline JniObjectT callStaticObjectMethod(const char *className, const char *methodName, const char *signature)
    {
        JniObjectT obj;
        obj.addToProtocol(QLatin1String("callStaticObjectMethod: ") + QLatin1String(className) + QLatin1Char(' ')
            + QLatin1String(methodName) + QLatin1Char(' ') + QLatin1String(signature) + QLatin1String(" ()"));
        return obj;
    }

    static inline JniObjectT getStaticObjectField(const char *className, const char *fieldName, const char *signature)
    {
        m_staticProtocol.push_back(QLatin1String("getStaticObjectField: ") + QLatin1String(className) + QLatin1Char(' ') + QLatin1String(fieldName) + QLatin1Char(' ') + QLatin1String(signature));
        return {};
    }

    template <typename T>
    static inline JniObjectT getStaticObjectField(const char *className, const char *fieldName)
    {
        m_staticProtocol.push_back(QLatin1String("getStaticObjectField<>: ") + QLatin1String(className) + QLatin1Char(' ') + QLatin1String(fieldName));
        return {};
    }

    template <typename T>
    static inline T getStaticField(const char *className, const char *fieldName)
    {
        m_staticProtocol.push_back(QLatin1String("getStaticField<>: ") + QLatin1String(className) + QLatin1Char(' ') + QLatin1String(fieldName) + QLatin1Char(' ') + QLatin1String(KAndroidExtras::Jni::signature<T>()));
        return {};
    }

    inline void setObjectProperty(const QByteArray &name, jobject value)
    {
        JniObjectT o;
        o.setData(value);
        setProperty(name, QVariant::fromValue(o));
    }
};

}
}

#endif
