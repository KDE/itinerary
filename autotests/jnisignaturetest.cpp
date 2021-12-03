/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kandroidextras/androidtypes.h>
#include <kandroidextras/jnisignature.h>
#include <kandroidextras/javatypes.h>

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

        // Jni::signature<java::lang::String[]()>() would be invalid C++, so we need alternatives for array returns
        QCOMPARE((const char*)Jni::signature<java::lang::String*()>(), "()[Ljava/lang/String;");
        QCOMPARE((const char*)Jni::signature<Jni::Array<java::lang::String>()>(), "()[Ljava/lang/String;");

        QCOMPARE((const char*)Jni::signature<jlong()>(), "()J");
    }

    void testTypeName()
    {
        QCOMPARE((const char*)Jni::typeName<android::provider::OpenableColumns>(), "android/provider/OpenableColumns");
        QCOMPARE((const char*)Jni::typeName<android::Manifest_permission>(), "android/Manifest$permission");
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
