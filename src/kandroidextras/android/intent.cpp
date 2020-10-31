/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "intent.h"
#include "uri.h"

#include <KAndroidExtras/JniSignature>
#include <KAndroidExtras/JniTypes>

#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#include <QUrl>

using namespace KAndroidExtras;

Intent::Intent()
{
    m_intent = QAndroidJniObject(Jni::typeName<android::content::Intent>());
}

Intent::Intent(const QAndroidJniObject &intent)
    : m_intent(intent)
{
}

Intent::~Intent() = default;

void Intent::addCategory(const QAndroidJniObject &category)
{
    m_intent.callObjectMethod("addCategory", Jni::signature<android::content::Intent(java::lang::String)>(), category.object());
}

void Intent::addFlags(jint flags)
{
    m_intent.callObjectMethod("addFlags", Jni::signature<android::content::Intent(int)>(), flags);
}

QUrl Intent::getData() const
{
    if (!m_intent.isValid()) {
        return {};
    }
    const auto uri = m_intent.callObjectMethod("getData", Jni::signature<android::net::Uri()>());
    return Uri::toUrl(uri);
}

QString Intent::getAction() const
{
    return m_intent.callObjectMethod("getAction", Jni::signature<java::lang::String()>()).toString();
}

void Intent::setAction(const QAndroidJniObject &action)
{
    m_intent.callObjectMethod("setAction", Jni::signature<android::content::Intent(java::lang::String)>(), action.object());
}

void Intent::setData(const QUrl& url)
{
    const auto uri = Uri::fromUrl(url);
    setData(uri);
}

void Intent::setData(const QAndroidJniObject& uri)
{
    m_intent.callObjectMethod("setData", Jni::signature<android::content::Intent(android::net::Uri)>(), uri.object());
}

QString Intent::getType() const
{
    return m_intent.callObjectMethod("getType", Jni::signature<java::lang::String()>()).toString();
}

void Intent::setType(const QString &type)
{
    m_intent.callObjectMethod("setType", Jni::signature<android::content::Intent(java::lang::String)>(), QAndroidJniObject::fromString(type).object());
}

Intent::operator QAndroidJniObject() const
{
    return m_intent;
}

template <typename T>
QAndroidJniObject Intent::getObjectExtra(const char *methodName, const QAndroidJniObject &name) const
{
    return m_intent.callObjectMethod(methodName, Jni::signature<T(java::lang::String)>(), name.object());
}

QString Intent::getStringExtra(const QAndroidJniObject &name) const
{
    return getObjectExtra<java::lang::String>("getStringExtra", name).toString();
}

QStringList Intent::getStringArrayExtra(const QAndroidJniObject &name) const
{
    const auto extra = getObjectExtra<Jni::Array<java::lang::String>>("getStringArrayExtra", name);
    const auto array = static_cast<jobjectArray>(extra.object());

    QAndroidJniEnvironment env;
    const auto size = env->GetArrayLength(array);

    QStringList r;
    r.reserve(size);
    for (auto i = 0; i < size; ++i) {
        r.push_back(QAndroidJniObject::fromLocalRef(env->GetObjectArrayElement(array, i)).toString());;
    }
    return r;
}
