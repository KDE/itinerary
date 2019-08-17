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

#include <../src/kandroidextras/jnisignature.h>
#include <../src/kandroidextras/jnitypes.h>

#include <QtTest/qtest.h>

using namespace KAndroidExtras;

class JniSignatureTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSignature()
    {
        QCOMPARE((const char*)Jni::signature<bool>(), "Z");
        QCOMPARE((const char*)Jni::signature<bool()>(), "()Z");
        QCOMPARE((const char*)Jni::signature<void(float[])>(), "([F)V");
        QCOMPARE((const char*)Jni::signature<void(java::lang::String)>(), "(Ljava/lang/String;)V");
        QCOMPARE((const char*)Jni::signature<java::lang::String()>(), "()Ljava/lang/String;");
        QCOMPARE((const char*)Jni::signature<android::content::Intent(java::lang::String, bool[])>(), "(Ljava/lang/String;[Z)Landroid/content/Intent;");
        QCOMPARE((const char*)Jni::signature<android::database::Cursor(android::net::Uri, java::lang::String[], java::lang::String, java::lang::String[], java::lang::String)>(), "(Landroid/net/Uri;[Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;)Landroid/database/Cursor;");
    }

    void testTypeName()
    {
        QCOMPARE((const char*)Jni::typeName<android::provider::OpenableColumns>(), "android/provider/OpenableColumns");
    }

    void testImplementationDetails()
    {
        static_assert(Internal::static_strlen("Hello!") == 6, "");
        QCOMPARE(java::lang::String::jniName(), "java/lang/String");
        QCOMPARE(((const char*)Internal::staticStringFromJniType<java::lang::String, std::make_index_sequence<16>>::value()), "java/lang/String");
    }
};

QTEST_GUILESS_MAIN(JniSignatureTest)

#include "jnisignaturetest.moc"
