/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uri.h"

#include <KAndroidExtras/JniSignature>
#include <KAndroidExtras/JniTypes>

#include <QAndroidJniObject>
#include <QUrl>

using namespace KAndroidExtras;

QAndroidJniObject Uri::fromUrl(const QUrl &url)
{
    return QAndroidJniObject::callStaticObjectMethod(Jni::typeName<android::net::Uri>(), "parse", Jni::signature<android::net::Uri(java::lang::String)>(),
        QAndroidJniObject::fromString(url.toString()).object<jstring>());
}

QUrl Uri::toUrl(const QAndroidJniObject &uri)
{
    if (!uri.isValid()) {
        return QUrl();
    }
    return QUrl(uri.callObjectMethod("toString", Jni::signature<java::lang::String()>()).toString());
}
