/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kandroidextras/jniproperty.h>
#include <kandroidextras/jnitypes.h>
#include <kandroidextras/manifestpermission.h>

#include <QtTest/qtest.h>

using namespace KAndroidExtras;

class TestClass : android::content::Intent
{
    JNI_OBJECT(TestClass)

    JNI_CONSTANT(java::lang::String, ACTION_CREATE_DOCUMENT)
    JNI_CONSTANT(jint, FLAG_GRANT_READ_URI_PERMISSION)
    JNI_CONSTANT(android::net::Uri, OBJECT_TYPE_PROPERTY)

    JNI_PROPERTY(java::lang::String, myStringField)
    JNI_PROPERTY(int, myIntField)
    JNI_PROPERTY(android::net::Uri, myUriField)

    inline QAndroidJniObject handle() const { return m_handle; }
private:
    QAndroidJniObject m_handle;
};

static_assert(sizeof(TestClass) == sizeof(QAndroidJniObject));

class JniPropertyTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPropertyRead()
    {
#ifndef Q_OS_ANDROID
        const QString p1 = TestClass::ACTION_CREATE_DOCUMENT;
        Q_UNUSED(p1)
        int32_t p2 = TestClass::FLAG_GRANT_READ_URI_PERMISSION;
        Q_UNUSED(p2)
        QAndroidJniObject p3 = TestClass::OBJECT_TYPE_PROPERTY;
        Q_UNUSED(p3)

        QCOMPARE(QAndroidJniObject::m_staticProtocol.size(), 3);
        QCOMPARE(QAndroidJniObject::m_staticProtocol.at(0), QLatin1String("getStaticObjectField: android/content/Intent ACTION_CREATE_DOCUMENT Ljava/lang/String;"));
        QCOMPARE(QAndroidJniObject::m_staticProtocol.at(1), QLatin1String("getStaticField<>: android/content/Intent FLAG_GRANT_READ_URI_PERMISSION I"));
        QCOMPARE(QAndroidJniObject::m_staticProtocol.at(2), QLatin1String("getStaticObjectField: android/content/Intent OBJECT_TYPE_PROPERTY Landroid/net/Uri;"));

        const QString p4 = ManifestPermission::READ_CALENDAR;
        Q_UNUSED(p4)
        QCOMPARE(QAndroidJniObject::m_staticProtocol.at(3), QLatin1String("getStaticObjectField: android/Manifest$permission READ_CALENDAR Ljava/lang/String;"));

        TestClass obj;
        const QString foo = obj.myStringField;
        obj.myStringField = foo;
        const int bar = obj.myIntField;
        obj.myIntField = 42;
        const QAndroidJniObject bla = obj.myUriField;
        obj.myUriField = bla;

        QCOMPARE(obj.handle().protocol().size(), 6);
        QCOMPARE(obj.handle().protocol().at(0), QLatin1String("getObjectField: myStringField Ljava/lang/String;"));
        QCOMPARE(obj.handle().protocol().at(1), QLatin1String("setField: myStringField Ljava/lang/String;"));
        QCOMPARE(obj.handle().protocol().at(2), QLatin1String("getField: myIntField I"));
        QCOMPARE(obj.handle().protocol().at(3), QLatin1String("setField: myIntField I"));
        QCOMPARE(obj.handle().protocol().at(4), QLatin1String("getObjectField: myUriField Landroid/net/Uri;"));
        QCOMPARE(obj.handle().protocol().at(5), QLatin1String("setField: myUriField Landroid/net/Uri;"));
#endif
    }
};

QTEST_GUILESS_MAIN(JniPropertyTest)

#include "jnipropertytest.moc"
