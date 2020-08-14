/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "intent.h"
#include "uri.h"
#include "jnitypes.h"
#include "jnisignature.h"

#include <QAndroidJniObject>
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

void Intent::setType(const QString &type)
{
    m_intent.callObjectMethod("setType", Jni::signature<android::content::Intent(java::lang::String)>(), QAndroidJniObject::fromString(type).object());
}

Intent::operator QAndroidJniObject() const
{
    return m_intent;
}
