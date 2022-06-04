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

class TestClass
{
    JNI_OBJECT(TestClass, android::content::Intent)
public:
    TestClass() = default;
    JNI_CONSTRUCTOR(TestClass, android::content::Intent)
    JNI_METHOD(java::lang::String, getName)
    JNI_METHOD(void, setName, java::lang::String)
    JNI_METHOD(jint, getFlags)
    JNI_METHOD(void, setFlags, jint)
    JNI_METHOD(void, start)
    JNI_METHOD(bool, setCoordinates, jfloat, jfloat)
    JNI_METHOD(void, startIntent, android::content::Intent)
    JNI_METHOD(android::content::Intent, getIntent)
    JNI_METHOD(Jni::Array<java::lang::String>, getStringList)
    JNI_METHOD(Jni::Array<jshort>, getShortList)

    // overloads
    JNI_METHOD(void, overloaded)
    JNI_METHOD(void, overloaded, jint)
    JNI_METHOD(void, overloaded, java::lang::String, jboolean)
    JNI_METHOD(jint, overloaded, jlong, java::lang::String, Jni::Array<jshort>)
    JNI_METHOD(Jni::Array<jshort>, overloaded, Intent)

    JNI_PROPERTY(java::lang::String, name)

    JNI_STATIC_METHOD(void, noRetNoArg)
    JNI_STATIC_METHOD(jlong, retNoArg)
    JNI_STATIC_METHOD(void, noRetArg, java::lang::String)
    JNI_STATIC_METHOD(android::content::Intent, retArg, bool)

    friend class JniMethodTest;
};

class JniMethodTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testMethodCalls()
    {
#ifndef Q_OS_ANDROID
        TestClass obj;
        QString s = obj.getName();
        Q_UNUSED(s);
        obj.setName(QStringLiteral("bla"));
         // explicit cast needed when coming from untyped JNI handler
        obj.setName(Jni::Object<java::lang::String>(QAndroidJniObject::fromString(QStringLiteral("bla"))));
        int i = obj.getFlags();
        Q_UNUSED(i);
        obj.setFlags(42);
        obj.start();
        bool b = obj.setCoordinates(0.0f, 0.0f);
        Q_UNUSED(b)

        // implicit conversion
        b = obj.setCoordinates(0.0, 0.0);
        // implicit conversion, and must not copy the property wrapper
        obj.setName(obj.name);
        // implicit conversion from manual wrappers
        Intent intent;
        obj.startIntent(intent);
        // returning a non-wrapped type
        QAndroidJniObject j = obj.getIntent();
        // lvalue QAndroidJniObject argument needs explicit cast
        obj.setName(Jni::Object<java::lang::String>(j));
        // implicit conversion from a static property wrapper
        obj.setFlags(Intent::FLAG_GRANT_READ_URI_PERMISSION);

        // avoiding type conversion on return values
        j = obj.getName();
        // call chaining with JNI handle pass-through
        obj.setName(obj.getName());

        // conversion of array return types
        j = obj.getStringList();
        QStringList l = obj.getStringList();
        std::vector<QString> l2 = obj.getStringList();
        std::vector<jshort> l3 = obj.getShortList();

        // nullptr arguments
        obj.startIntent(nullptr);

        // overloaded methods
        obj.overloaded();
        obj.overloaded(42);
        obj.overloaded(QStringLiteral("hello"), true);
        Jni::Array<jshort> l4 = obj.overloaded(intent);
        obj.overloaded(23, QStringLiteral("world"), l4);


        QCOMPARE(obj.jniHandle().protocol().size(), 27);
        QCOMPARE(obj.jniHandle().protocol()[0], QLatin1String("callObjectMethod: getName ()Ljava/lang/String; ()"));
        QCOMPARE(obj.jniHandle().protocol()[1], QLatin1String("callMethod: setName (Ljava/lang/String;)V (o)"));
        QCOMPARE(obj.jniHandle().protocol()[2], QLatin1String("callMethod: setName (Ljava/lang/String;)V (o)"));
        QCOMPARE(obj.jniHandle().protocol()[3], QLatin1String("callMethod: getFlags ()I ()"));
        QCOMPARE(obj.jniHandle().protocol()[4], QLatin1String("callMethod: setFlags (I)V (I)"));
        QCOMPARE(obj.jniHandle().protocol()[5], QLatin1String("callMethod: start ()V ()"));
        QCOMPARE(obj.jniHandle().protocol()[6], QLatin1String("callMethod: setCoordinates (FF)Z (FF)"));

        QCOMPARE(obj.jniHandle().protocol()[7], QLatin1String("callMethod: setCoordinates (FF)Z (FF)"));
        QCOMPARE(obj.jniHandle().protocol()[8], QLatin1String("getObjectField: name Ljava/lang/String;"));
        QCOMPARE(obj.jniHandle().protocol()[9], QLatin1String("callMethod: setName (Ljava/lang/String;)V (o)"));
        QCOMPARE(obj.jniHandle().protocol()[10], QLatin1String("callMethod: startIntent (Landroid/content/Intent;)V (o)"));
        QCOMPARE(obj.jniHandle().protocol()[11], QLatin1String("callObjectMethod: getIntent ()Landroid/content/Intent; ()"));
        QCOMPARE(obj.jniHandle().protocol()[12], QLatin1String("callMethod: setName (Ljava/lang/String;)V (o)"));
        QCOMPARE(obj.jniHandle().protocol()[13], QLatin1String("callMethod: setFlags (I)V (I)"));
        QCOMPARE(obj.jniHandle().protocol()[14], QLatin1String("callObjectMethod: getName ()Ljava/lang/String; ()"));
        QCOMPARE(obj.jniHandle().protocol()[15], QLatin1String("callObjectMethod: getName ()Ljava/lang/String; ()"));
        QCOMPARE(obj.jniHandle().protocol()[16], QLatin1String("callMethod: setName (Ljava/lang/String;)V (o)"));
        QCOMPARE(obj.jniHandle().protocol()[17], QLatin1String("callObjectMethod: getStringList ()[Ljava/lang/String; ()"));
        QCOMPARE(obj.jniHandle().protocol()[18], QLatin1String("callObjectMethod: getStringList ()[Ljava/lang/String; ()"));
        QCOMPARE(obj.jniHandle().protocol()[19], QLatin1String("callObjectMethod: getStringList ()[Ljava/lang/String; ()"));
        QCOMPARE(obj.jniHandle().protocol()[20], QLatin1String("callObjectMethod: getShortList ()[S ()"));
        QCOMPARE(obj.jniHandle().protocol()[21], QLatin1String("callMethod: startIntent (Landroid/content/Intent;)V (o)"));

        QCOMPARE(obj.jniHandle().protocol()[22], QLatin1String("callMethod: overloaded ()V ()"));
        QCOMPARE(obj.jniHandle().protocol()[23], QLatin1String("callMethod: overloaded (I)V (I)"));
        QCOMPARE(obj.jniHandle().protocol()[24], QLatin1String("callMethod: overloaded (Ljava/lang/String;Z)V (oZ)"));
        QCOMPARE(obj.jniHandle().protocol()[25], QLatin1String("callObjectMethod: overloaded (Landroid/content/Intent;)[S (o)"));
        QCOMPARE(obj.jniHandle().protocol()[26], QLatin1String("callMethod: overloaded (JLjava/lang/String;[S)I (Joo)"));

        // ctor call
        obj = TestClass(intent);
        QCOMPARE(obj.jniHandle().protocol().size(), 1);
        QCOMPARE(obj.jniHandle().protocol()[0], QLatin1String("ctor: android/content/Intent (Landroid/content/Intent;)V (o)"));
#if 0
        // stuff that must not compile
        obj.setName(42);
        obj.setFlags(QStringLiteral("42"));
        s = obj.getStringList();
        obj.setFlags(nullptr);
#endif
#endif
    }

    void testStaticCalls()
    {
#ifndef Q_OS_ANDROID
        QAndroidJniObject::m_staticProtocol.clear();

        TestClass::noRetNoArg();
        TestClass::noRetArg(QStringLiteral("test"));
        TestClass::retNoArg();
        QAndroidJniObject o = TestClass::retArg(true);

        QCOMPARE(QAndroidJniObject::m_staticProtocol.size(), 3);
        QCOMPARE(QAndroidJniObject::m_staticProtocol[0], QLatin1String("callStaticMethod: android/content/Intent noRetNoArg ()V ()"));
        QCOMPARE(QAndroidJniObject::m_staticProtocol[1], QLatin1String("callStaticMethod: android/content/Intent noRetArg (Ljava/lang/String;)V (o)"));
        QCOMPARE(QAndroidJniObject::m_staticProtocol[2], QLatin1String("callStaticMethod: android/content/Intent retNoArg ()J ()"));
        QCOMPARE(o.protocol().at(0), QLatin1String("callStaticObjectMethod: android/content/Intent retArg (Z)Landroid/content/Intent; (Z)"));
#endif
    }
};

QTEST_GUILESS_MAIN(JniMethodTest)

#include "jnimethodtest.moc"
