/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef FAKE_QANDROIDJNIOBJECT_H
#define FAKE_QANDROIDJNIOBJECT_H

#include "kandroidextras_export.h"
#include "jni.h"

#include <kandroidextras/jnisignature.h>
#include <kandroidextras/jnitypes.h>

#include <QExplicitlySharedDataPointer>
#include <QStringList>

class QAndroidJniObjectPrivate;

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
}
}

/** Mock object for QAndroidJniObject outside of Android, for automated testing. */
class KANDROIDEXTRAS_EXPORT QAndroidJniObject {
public:
    QAndroidJniObject();
    QAndroidJniObject(const QAndroidJniObject&);
    ~QAndroidJniObject();
    QAndroidJniObject& operator=(const QAndroidJniObject&);

    QAndroidJniObject(const char *className);
    QAndroidJniObject(const char *className, const char *signature, ...);

    bool isValid() const { return true; }

    static QAndroidJniObject fromString(const QString &s) { Q_UNUSED(s); return {}; }
    static QAndroidJniObject fromLocalRef(jobject o)
    {
        QAndroidJniObject obj;
        obj.addToProtocol(QLatin1String("ctor: ") + QString::number((qulonglong)o));
        return obj;
    }
    QString toString() const { return protocol().join(QLatin1Char('\n')); }

    jobject object() const { return nullptr; }
    template <typename T>
    T object() const { return {}; }

    template <typename T, typename ...Args>
    T callMethod(const char *methodName, const char *signature, Args...) const
    {
        const QString s = QLatin1String("callMethod: ") + QLatin1String(methodName) + QLatin1Char(' ') + QLatin1String(signature) + QLatin1String(" (")
            + (...+QLatin1String(KAndroidExtras::Internal::argTypeToString<Args>())) + QLatin1Char(')');
        addToProtocol(s);
        if constexpr (!std::is_same_v<T, void>) {
            return {};
        }
    }
    template <typename T>
    T callMethod(const char *methodName, const char *signature) const
    {
        const QString s = QLatin1String("callMethod: ") + QLatin1String(methodName) + QLatin1Char(' ') + QLatin1String(signature) + QLatin1String(" ()");
        addToProtocol(s);
        if constexpr (!std::is_same_v<T, void>) {
            return {};
        }
    }

    template <typename ...Args>
    QAndroidJniObject callObjectMethod(const char *methodName, const char *signature, Args...) const
    {
        const QString s = QLatin1String("callObjectMethod: ") + QLatin1String(methodName) + QLatin1Char(' ') + QLatin1String(signature) + QLatin1String(" (")
            + (...+QLatin1String(KAndroidExtras::Internal::argTypeToString<Args>())) + QLatin1Char(')');
        addToProtocol(s);

        QAndroidJniObject obj;
        obj.setProtocol(protocol());
        return obj;
    }
    QAndroidJniObject callObjectMethod(const char *methodName, const char *signature) const
    {
        addToProtocol(QLatin1String("callObjectMethod: ") + QLatin1String(methodName) + QLatin1Char(' ') + QLatin1String(signature) + QLatin1String(" ()"));

        QAndroidJniObject obj;
        obj.setProtocol(protocol());
        return obj;
    }

    template <typename T>
    T getField(const char *fieldName) const
    {
        addToProtocol(QLatin1String("getField: ") + QLatin1String(fieldName) + QLatin1Char(' ') + QLatin1String(KAndroidExtras::Jni::signature<T>()));
        return {};
    }

    QAndroidJniObject getObjectField(const char *fieldName, const char *signature) const
    {
        addToProtocol(QLatin1String("getObjectField: ") + QLatin1String(fieldName) + QLatin1Char(' ') + QLatin1String(signature));
        return {};
    }

    template <typename T>
    void setField(const char *fieldName, const char *signature, T value)
    {
        Q_UNUSED(value);
        addToProtocol(QLatin1String("setField: ") + QLatin1String(fieldName) + QLatin1Char(' ') + QLatin1String(signature));
    }

    static QAndroidJniObject callStaticObjectMethod(const char *methodName, const char *signature, ...)
    {
        QAndroidJniObject obj;
        obj.addToProtocol(QLatin1String("callStaticObjectMethod: ") + QLatin1String(methodName) + QLatin1Char(' ') + QLatin1String(signature));
        return obj;
    }

    static QAndroidJniObject getStaticObjectField(const char *className, const char *fieldName, const char *signature)
    {
        m_staticProtocol.push_back(QLatin1String("getStaticObjectField: ") + QLatin1String(className) + QLatin1Char(' ') + QLatin1String(fieldName) + QLatin1Char(' ') + QLatin1String(signature));
        return {};
    }

    template <typename T>
    static QAndroidJniObject getStaticObjectField(const char *className, const char *fieldName)
    {
        m_staticProtocol.push_back(QLatin1String("getStaticObjectField<>: ") + QLatin1String(className) + QLatin1Char(' ') + QLatin1String(fieldName));
        return {};
    }

    template <typename T>
    static T getStaticField(const char *className, const char *fieldName)
    {
        m_staticProtocol.push_back(QLatin1String("getStaticField<>: ") + QLatin1String(className) + QLatin1Char(' ') + QLatin1String(fieldName) + QLatin1Char(' ') + QLatin1String(KAndroidExtras::Jni::signature<T>()));
        return {};
    }

    static QStringList m_staticProtocol;

    QStringList protocol() const;
    void addToProtocol(const QString &line) const;

private:
    void setProtocol(const QStringList &protocol);
    QExplicitlySharedDataPointer<QAndroidJniObjectPrivate> d;
};

#endif
