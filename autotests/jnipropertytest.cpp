/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <../src/kandroidextras/jniproperty.h>
#include <../src/kandroidextras/jnitypes.h>

#include <QtTest/qtest.h>

using namespace KAndroidExtras;

class TestClass : Jni::Wrapper<android::content::Intent>
{
public:
     JNI_CONSTANT(java::lang::String, ACTION_CREATE_DOCUMENT)
     JNI_CONSTANT(jint, FLAG_GRANT_READ_URI_PERMISSION)
     JNI_CONSTANT(android::net::Uri, OBJECT_TYPE_PROPERTY)
};

class JniPropertyTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPropertyRead()
    {
#ifndef Q_OS_ANDROID
        const QString p1 = TestClass::ACTION_CREATE_DOCUMENT;
        Q_UNUSED(p1);
        int32_t p2 = TestClass::FLAG_GRANT_READ_URI_PERMISSION;
        Q_UNUSED(p2);
        QAndroidJniObject p3 = TestClass::OBJECT_TYPE_PROPERTY;
        Q_UNUSED(p3);

        QCOMPARE(QAndroidJniObject::m_staticProtocol.size(), 3);
        QCOMPARE(QAndroidJniObject::m_staticProtocol.at(0), QLatin1String("getStaticObjectField<>: android/content/Intent ACTION_CREATE_DOCUMENT"));
        QCOMPARE(QAndroidJniObject::m_staticProtocol.at(1), QLatin1String("getStaticField<>: android/content/Intent FLAG_GRANT_READ_URI_PERMISSION I"));
        QCOMPARE(QAndroidJniObject::m_staticProtocol.at(2), QLatin1String("getStaticObjectField: android/content/Intent OBJECT_TYPE_PROPERTY Landroid/net/Uri;"));
#endif
    }
};

QTEST_GUILESS_MAIN(JniPropertyTest)

#include "jnipropertytest.moc"
