/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KAndroidExtras/Context>
#include <KAndroidExtras/ContentResolver>

#include <QAndroidJniObject>
#include <QtTest/qtest.h>

using namespace KAndroidExtras;

class AndroidWrapperTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testContext()
    {
#ifndef Q_OS_ANDROID
        const auto pn = Context::getPackageName();
        QCOMPARE(pn.protocol().size(), 2);
        QCOMPARE(pn.protocol().at(0), QLatin1String("global androidContext()"));
        QCOMPARE(pn.protocol().at(1), QLatin1String("callObjectMethod: getPackageName ()Ljava/lang/String;"));
#endif
    }

    void testContentResolver()
    {
#ifndef Q_OS_ANDROID
        QCOMPARE(ContentResolver::fileName(QUrl()), QLatin1String(
            "global androidContext()\n"
            "callObjectMethod: getContentResolver ()Landroid/content/ContentResolver;\n"
            "callObjectMethod: query (Landroid/net/Uri;[Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;)Landroid/database/Cursor;\n"
            "callMethod: getColumnIndex (Ljava/lang/String;)I\n"
            "callMethod: moveToFirst ()Z\n"
            "callObjectMethod: getString (I)Ljava/lang/String;"
        ));
        QCOMPARE(QAndroidJniObject::m_staticProtocol.size(), 1);
        QCOMPARE(QAndroidJniObject::m_staticProtocol.at(0), QLatin1String("getStaticObjectField: android/provider/OpenableColumns DISPLAY_NAME Ljava/lang/String;"));
#endif
    }
};

QTEST_GUILESS_MAIN(AndroidWrapperTest)

#include "androidwrappertest.moc"
