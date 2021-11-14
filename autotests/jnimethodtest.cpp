/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kandroidextras/jnimethod.h>
#include <kandroidextras/jnitypes.h>
#include <kandroidextras/intent.h>
#include <kandroidextras/javatypes.h>

#include <QtTest/qtest.h>

using namespace KAndroidExtras;

class TestClass : android::content::Intent
{
    JNI_OBJECT(TestClass)
public:
    JNI_METHOD(java::lang::String, getName)
    JNI_METHOD(void, setName, java::lang::String)
    JNI_METHOD(jint, getFlags)
    JNI_METHOD(void, setFlags, jint)
    JNI_METHOD(void, start)
    JNI_METHOD(bool, setCoordinates, jfloat, jfloat)

    inline QAndroidJniObject handle() const { return m_handle; }
private:
    QAndroidJniObject m_handle;
};

class JniMethodTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testArgumentConversion()
    {
        static_assert(Internal::is_argument_compatible<jint, jint>::value);
        static_assert(Internal::is_argument_compatible<java::lang::String, QAndroidJniObject>::value);
        static_assert(Internal::is_argument_compatible<java::lang::String, QString>::value);

        static_assert(!Internal::is_argument_compatible<jint, jdouble>::value);
        static_assert(!Internal::is_argument_compatible<jint, QAndroidJniObject>::value);
        static_assert(!Internal::is_argument_compatible<java::lang::String, QUrl>::value);

        static_assert(Internal::is_call_compatible<jint>::with<jint>::value);
        static_assert(Internal::is_call_compatible<java::lang::String, jfloat>::with<QAndroidJniObject, float>::value);
        static_assert(!Internal::is_call_compatible<java::lang::String, jfloat>::with<float, QAndroidJniObject>::value);
        static_assert(Internal::is_call_compatible<java::lang::String, java::lang::String>::with<QString, QAndroidJniObject>::value);
        static_assert(Internal::is_call_compatible<java::lang::String, java::lang::String>::with<QAndroidJniObject, QString>::value);
    }

    void testMethodCalls()
    {
#ifndef Q_OS_ANDROID
        TestClass obj;
        QString s = obj.getName();
        Q_UNUSED(s);
        obj.setName(QStringLiteral("bla"));
        obj.setName(QAndroidJniObject::fromString(QStringLiteral("bla")));
        int i = obj.getFlags();
        Q_UNUSED(i);
        obj.setFlags(42);
        obj.start();
        bool b = obj.setCoordinates(0.0f, 0.0f);
        Q_UNUSED(b)

        QCOMPARE(obj.handle().protocol().size(), 7);
        QCOMPARE(obj.handle().protocol()[0], QLatin1String("callObjectMethod: getName ()Ljava/lang/String;"));
        QCOMPARE(obj.handle().protocol()[1], QLatin1String("callMethod: setName (Ljava/lang/String;)V"));
        QCOMPARE(obj.handle().protocol()[2], QLatin1String("callMethod: setName (Ljava/lang/String;)V"));
        QCOMPARE(obj.handle().protocol()[3], QLatin1String("callMethod: getFlags ()I"));
        QCOMPARE(obj.handle().protocol()[4], QLatin1String("callMethod: setFlags (I)V"));
        QCOMPARE(obj.handle().protocol()[5], QLatin1String("callMethod: start ()V"));
        QCOMPARE(obj.handle().protocol()[6], QLatin1String("callMethod: setCoordinates (FF)Z"));

#if 0
        // stuff that must not compile
        obj.setName(42);
        obj.setFlags(QStringLiteral("42"));
#endif
#endif
    }
};

QTEST_GUILESS_MAIN(JniMethodTest)

#include "jnimethodtest.moc"
