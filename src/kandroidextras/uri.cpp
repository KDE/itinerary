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

#include "uri.h"
#include "jnisignature.h"
#include "jnitypes.h"

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
