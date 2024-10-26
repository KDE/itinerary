/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KAndroidExtras/JavaTypes>
#include <KAndroidExtras/JniArray>
#include <KAndroidExtras/JniProperty>
#include <KAndroidExtras/JniTypeTraits>
#include <KAndroidExtras/Uri>

#include <QtTest/qtest.h>

using namespace KAndroidExtras;

class TestClass
{
    JNI_OBJECT(TestClass, android::content::Context)
    JNI_PROPERTY(bool, myBoolProp)
};

class JniArrayTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testTypeTrait()
    {
        static_assert(!Jni::is_array<int>::value);
        static_assert(Jni::is_array<Jni::Array<int>>::value);
    }

    void testFromArray()
    {
#ifndef Q_OS_ANDROID
        JNIEnv::m_arrayLength = 3;

        QJniObject array;
        const auto a1 = Jni::fromArray<std::vector<QJniObject>>(array);
        QCOMPARE(a1.size(), 3);

        JNIEnv::m_arrayLength = 2;
        const auto a2 = Jni::fromArray<QStringList>(array);
        QCOMPARE(a2.size(), 2);
        QCOMPARE(a2, QStringList({QStringLiteral("ctor: 0"), QStringLiteral("ctor: 1")}));

        JNIEnv::m_arrayLength = 4;
        const auto a3 = Jni::fromArray<std::vector<int>>(array);
        QCOMPARE(a3.size(), 4);

        JNIEnv::m_arrayLength = 1;
        const auto a4 = Jni::fromArray<QList<QUrl>>(array);
        QCOMPARE(a4.size(), 1);
#endif
    }

    void testArrayWrapper()
    {
#ifndef Q_OS_ANDROID
        JNIEnv::m_arrayLength = 3;

        QJniObject array;
        // primitive types
        const auto a1 = Jni::Array<jint>(array);
        QCOMPARE(a1.size(), 3);
        QCOMPARE(std::distance(a1.begin(), a1.end()), 3);
        QCOMPARE(a1[0], 0);
        QCOMPARE(a1[2], 2);
        for (jint i : a1) {
            QVERIFY(i >= 0 && i < 3);
        }

        // complex types without wrappers
        const auto a2 = Jni::Array<java::lang::String>(array);
        JNIEnv::m_arrayLength = 2;
        QCOMPARE(a2.size(), 2);
        QCOMPARE(std::distance(a2.begin(), a2.end()), 2);
        QCOMPARE(a2[0].toString(), QLatin1StringView("ctor: 0"));
        QCOMPARE(a2[1].toString(), QLatin1StringView("ctor: 1"));
        for (const auto &i : a2) {
            QVERIFY(i.isValid());
        }
        for (const QString &i : a2) {
            QVERIFY(!i.isEmpty());
        }

        // complex type with custom wrapper
        static_assert(Jni::is_object_wrapper<TestClass>::value);
        JNIEnv::m_arrayLength = 1;
        const auto a3 = Jni::Array<TestClass>(array);
        QCOMPARE(a3.size(), 1);
        QCOMPARE(std::distance(a3.begin(), a3.end()), 1);
        const auto v = a3[0];
        bool b = a3[0].myBoolProp;
        for (const TestClass &i : a3) {
            bool b = i.myBoolProp; // verifies this is of the correct type
        }
#endif
    }

    void testArrayCreate()
    {
        auto a1 = Jni::Array<jshort>(10);
        auto a2 = Jni::Array<java::lang::String>(5);
        a2[2] = QStringLiteral("test");
        auto a3 = Jni::Array<java::lang::String>(3, QStringLiteral("test"));
    }
};

QTEST_GUILESS_MAIN(JniArrayTest)

#include "jniarraytest.moc"
