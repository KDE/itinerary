/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kandroidextras/jniproperty.h"
#include "kandroidextras/jnitypes.h"
#include "kandroidextras/manifestpermission.h"
#include "kandroidextras/intent.h"
#include "kandroidextras/uri.h"

#include <QtTest/qtest.h>

using namespace KAndroidExtras;

class TestClass
{
    TestClass() = default;
    JNI_OBJECT(TestClass, android::content::Intent)

    JNI_CONSTANT(java::lang::String, ACTION_CREATE_DOCUMENT)
    JNI_CONSTANT(jint, FLAG_GRANT_READ_URI_PERMISSION)
    JNI_CONSTANT(android::net::Uri, OBJECT_TYPE_PROPERTY)
    JNI_CONSTANT(bool, BOOL_PROPERTY)
//     JNI_CONSTANT(uint32_t, UNSIGNED_PROPERTY) // must not compile

    JNI_PROPERTY(java::lang::String, myStringField)
    JNI_PROPERTY(int, myIntField)
    JNI_PROPERTY(android::net::Uri, myUriField)
    JNI_PROPERTY(android::content::Intent, myIntentField)
    JNI_PROPERTY(bool, myBoolProperty)
//     JNI_PROPERTY(uint32_t, myUsignedProperty) // must not compile

public:
    friend class JniPropertyTest;
};

static_assert(sizeof(TestClass) == sizeof(QJniObject));

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
        QJniObject p3 = TestClass::OBJECT_TYPE_PROPERTY;
        Q_UNUSED(p3)
        bool b = TestClass::BOOL_PROPERTY;

        QCOMPARE(QJniObject::m_staticProtocol.size(), 4);
        QCOMPARE(QJniObject::m_staticProtocol.at(0), QLatin1StringView("getStaticObjectField: android/content/Intent ACTION_CREATE_DOCUMENT Ljava/lang/String;"));
        QCOMPARE(QJniObject::m_staticProtocol.at(1), QLatin1StringView("getStaticField<>: android/content/Intent FLAG_GRANT_READ_URI_PERMISSION I"));
        QCOMPARE(QJniObject::m_staticProtocol.at(2), QLatin1StringView("getStaticObjectField: android/content/Intent OBJECT_TYPE_PROPERTY Landroid/net/Uri;"));
        QCOMPARE(QJniObject::m_staticProtocol.at(3), QLatin1StringView("getStaticField<>: android/content/Intent BOOL_PROPERTY Z"));

        const QString p4 = ManifestPermission::READ_CALENDAR;
        Q_UNUSED(p4)
        QCOMPARE(QJniObject::m_staticProtocol.at(4), QLatin1StringView("getStaticObjectField: android/Manifest$permission READ_CALENDAR Ljava/lang/String;"));


        TestClass obj;
        const QString foo = obj.myStringField;
        obj.myStringField = foo;
        const int bar = obj.myIntField;
        Q_UNUSED(bar);
        obj.myIntField = 42;
        const QUrl url = obj.myUriField;
        obj.myUriField = url;
        const QJniObject bla = obj.myIntentField;
        obj.myIntentField = bla;
        b = obj.myBoolProperty;
        obj.myBoolProperty = b;

        QCOMPARE(obj.jniHandle().protocol().size(), 10);
        QCOMPARE(obj.jniHandle().protocol().at(0), QLatin1StringView("getObjectField: myStringField Ljava/lang/String;"));
        QCOMPARE(obj.jniHandle().protocol().at(1), QLatin1StringView("setField: myStringField Ljava/lang/String;"));
        QCOMPARE(obj.jniHandle().protocol().at(2), QLatin1StringView("getField: myIntField I"));
        QCOMPARE(obj.jniHandle().protocol().at(3), QLatin1StringView("setField: myIntField I"));
        QCOMPARE(obj.jniHandle().protocol().at(4), QLatin1StringView("getObjectField: myUriField Landroid/net/Uri;"));
        QCOMPARE(obj.jniHandle().protocol().at(5), QLatin1StringView("setField: myUriField Landroid/net/Uri;"));
        QCOMPARE(obj.jniHandle().protocol().at(6), QLatin1StringView("getObjectField: myIntentField Landroid/content/Intent;"));
        QCOMPARE(obj.jniHandle().protocol().at(7), QLatin1StringView("setField: myIntentField Landroid/content/Intent;"));
        QCOMPARE(obj.jniHandle().protocol().at(8), QLatin1StringView("getField: myBoolProperty Z"));
        QCOMPARE(obj.jniHandle().protocol().at(9), QLatin1StringView("setField: myBoolProperty Z"));
#endif
    }
};

QTEST_GUILESS_MAIN(JniPropertyTest)

#include "jnipropertytest.moc"
