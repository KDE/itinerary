/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KAndroidExtras/JniArray>
#include <KAndroidExtras/JniTypeTraits>
#include <KAndroidExtras/JavaTypes>
#include <KAndroidExtras/Uri>

#include <QtTest/qtest.h>

using namespace KAndroidExtras;

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

        QAndroidJniObject array;
        const auto a1 = Jni::fromArray<std::vector<QAndroidJniObject>>(array);
        QCOMPARE(a1.size(), 3);

        JNIEnv::m_arrayLength = 2;
        const auto a2 = Jni::fromArray<QStringList>(array);
        QCOMPARE(a2.size(), 2);
        QCOMPARE(a2, QStringList({ QStringLiteral("ctor: 0"), QStringLiteral("ctor: 1") }));

        JNIEnv::m_arrayLength = 4;
        const auto a3 = Jni::fromArray<std::vector<int>>(array);
        QCOMPARE(a3.size(), 4);

        JNIEnv::m_arrayLength = 1;
        const auto a4 = Jni::fromArray<QVector<QUrl>>(array);
        QCOMPARE(a4.size(), 1);
#endif
    }

    void testArrayWrapper()
    {
#ifndef Q_OS_ANDROID
        JNIEnv::m_arrayLength = 3;

        QAndroidJniObject array;
        // basic types
        const auto a1 = Jni::Array<jint>(array);
        QCOMPARE(a1.size(), 3);
        QCOMPARE(std::distance(a1.begin(), a1.end()), 3);
        QCOMPARE(a1[0], 0);
        QCOMPARE(a1[2], 2);
        for (jint i : a1) {
            QVERIFY(i >= 0 && i < 3);
        }

        // complex types
        const auto a2 = Jni::Array<java::lang::String>(array);
        JNIEnv::m_arrayLength = 2;
        QCOMPARE(a2.size(), 2);
        QCOMPARE(std::distance(a2.begin(), a2.end()), 2);
        QCOMPARE(a2[0].toString(), QLatin1String("ctor: 0"));
        QCOMPARE(a2[1].toString(), QLatin1String("ctor: 1"));
        for (const auto &i : a2) {
            QVERIFY(i.isValid());
        }

#endif
    }
};

QTEST_GUILESS_MAIN(JniArrayTest)

#include "jniarraytest.moc"
