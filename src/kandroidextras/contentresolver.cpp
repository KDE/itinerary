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

#include "contentresolver.h"
#include "uri.h"

#include <QtAndroid>
#include <QAndroidJniObject>

#include <QString>
#include <QUrl>

using namespace KAndroidExtras;

QAndroidJniObject ContentResolver::get()
{
    return QtAndroid::androidContext().callObjectMethod("getContentResolver", "()Landroid/content/ContentResolver;");
}

QString ContentResolver::mimeType(const QUrl &url)
{
    auto cs = ContentResolver::get();
    const auto uri = Uri::fromUrl(url);
    auto mt = cs.callObjectMethod("getType", "(Landroid/net/Uri;)Ljava/lang/String;", uri.object<jobject>());
    return mt.toString();
}

QString ContentResolver::fileName(const QUrl &url)
{
    auto cs = ContentResolver::get();
    const auto uri = Uri::fromUrl(url);

    // query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder)
    auto cursor = cs.callObjectMethod("query", "(Landroid/net/Uri;[Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;)Landroid/database/Cursor;", uri.object<jobject>(), 0, 0, 0, 0);

    const auto DISPLAY_NAME = QAndroidJniObject::getStaticObjectField<jstring>("android/provider/OpenableColumns", "DISPLAY_NAME");
    const auto nameIndex = cursor.callMethod<jint>("getColumnIndex", "(Ljava/lang/String;)I", DISPLAY_NAME.object<jobject>());
    cursor.callMethod<jboolean>("moveToFirst", "()Z");
    return cursor.callObjectMethod("getString", "(I)Ljava/lang/String;", nameIndex).toString();
}
