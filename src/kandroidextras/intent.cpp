/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

QAndroidJniObject Intent::ACTION_CREATE_DOCUMENT()
{
    return QAndroidJniObject::getStaticObjectField<jstring>(Jni::typeName<android::content::Intent>(), "ACTION_CREATE_DOCUMENT");
}

QAndroidJniObject Intent::ACTION_OPEN_DOCUMENT()
{
    return QAndroidJniObject::getStaticObjectField<jstring>(Jni::typeName<android::content::Intent>(), "ACTION_OPEN_DOCUMENT");
}

QAndroidJniObject Intent::ACTION_VIEW()
{
    return QAndroidJniObject::getStaticObjectField<jstring>(Jni::typeName<android::content::Intent>(), "ACTION_VIEW");
}

QAndroidJniObject Intent::CATEGORY_OPENABLE()
{
    return QAndroidJniObject::getStaticObjectField<jstring>(Jni::typeName<android::content::Intent>(), "CATEGORY_OPENABLE");
}
