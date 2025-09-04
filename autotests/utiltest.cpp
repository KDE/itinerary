/*
    SPDX-FileCopyrightText: Ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "util.h"

#include <QtTest/qtest.h>

using namespace Qt::Literals;

class UtilTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testIsRichText_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<bool>("richText");

        QTest::newRow("empty") << QString() << false;
        QTest::newRow("plain") << u"Frankfurt (Main) Hbf"_s << false;
        QTest::newRow("link") << u"<a href=\"https://maps.apple.com/?q=Frankfurt+%28Main%29+Hbf&ll=50.107149%2C8.663785\">Frankfurt (Main) Hbf</a>"_s << true;
        QTest::newRow("format") << u"bla <b>bla</b> bla"_s << true;
        QTest::newRow("entity") << u"Via: &lt;1080&gt;(L*EF*FD/SDL*WOB*H*GOE*KS)"_s << true;
    }

    void testIsRichText()
    {
        QFETCH(QString, input);
        QFETCH(bool, richText);

        QCOMPARE(Util::isRichText(input), richText);
    }
};

QTEST_APPLESS_MAIN(UtilTest)

#include "utiltest.moc"
