/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <publictransportmatcher.h>

#include <KPublicTransport/Line>

#include <QtTest/qtest.h>
#include <QStandardPaths>
#include <QTimeZone>

#define s(x) QStringLiteral(x)

class PublicTransportMatcherTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testMatchRoute()
    {
        KPublicTransport::Route route;
        KPublicTransport::Line line;
        line.setName(s("RE 13"));
        route.setLine(line);
        QVERIFY(PublicTransportMatcher::isSameRoute(route, s("RE"), s("13")));
        QVERIFY(PublicTransportMatcher::isSameRoute(route, QString(), s("RE13")));
        QVERIFY(!PublicTransportMatcher::isSameRoute(route, s("RE"), s("20072")));

        route.setName(s("RE 20072"));
        QVERIFY(PublicTransportMatcher::isSameRoute(route, s("RE"), s("13")));
        QVERIFY(PublicTransportMatcher::isSameRoute(route, s("RE"), s("20072")));
        QVERIFY(PublicTransportMatcher::isSameRoute(route, QString(), s("RE20072")));
    }
};

QTEST_GUILESS_MAIN(PublicTransportMatcherTest)

#include "publictransportmatchertest.moc"
