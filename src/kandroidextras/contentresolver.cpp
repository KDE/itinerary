/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "contentresolver.h"
#include "uri.h"
#include "jnisignature.h"
#include "jnitypes.h"

#include <QtAndroid>
#include <QAndroidJniObject>

#include <QString>
#include <QUrl>

using namespace KAndroidExtras;

QAndroidJniObject ContentResolver::get()
{
    return QtAndroid::androidContext().callObjectMethod("getContentResolver", Jni::signature<android::content::ContentResolver()>());
}

QString ContentResolver::mimeType(const QUrl &url)
{
    auto cs = ContentResolver::get();
    const auto uri = Uri::fromUrl(url);
    auto mt = cs.callObjectMethod("getType", Jni::signature<java::lang::String(android::net::Uri)>(), uri.object<jobject>());
    return mt.toString();
}

QString ContentResolver::fileName(const QUrl &url)
{
    auto cs = ContentResolver::get();
    const auto uri = Uri::fromUrl(url);

    // query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder)
    auto cursor = cs.callObjectMethod("query", Jni::signature<android::database::Cursor(android::net::Uri, java::lang::String[], java::lang::String, java::lang::String[], java::lang::String)>(), uri.object<jobject>(), 0, 0, 0, 0);

    const auto DISPLAY_NAME = QAndroidJniObject::getStaticObjectField<jstring>(Jni::typeName<android::provider::OpenableColumns>(), "DISPLAY_NAME");
    const auto nameIndex = cursor.callMethod<jint>("getColumnIndex", Jni::signature<int(java::lang::String)>(), DISPLAY_NAME.object<jobject>());
    cursor.callMethod<jboolean>("moveToFirst", Jni::signature<bool()>());
    return cursor.callObjectMethod("getString", Jni::signature<java::lang::String(int)>(), nameIndex).toString();
}
