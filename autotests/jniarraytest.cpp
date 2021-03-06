/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KAndroidExtras/JniArray>

#include <QtTest/qtest.h>

using namespace KAndroidExtras;

class JniArrayTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
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
#endif
    }
};

QTEST_GUILESS_MAIN(JniArrayTest)

#include "jniarraytest.moc"
