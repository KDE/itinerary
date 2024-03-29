/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "locationinformation.h"

#include <QtTest/qtest.h>

class LocationInformationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testTzCompare()
    {
        LocationInformation lhs;
        LocationInformation rhs;
        QVERIFY(lhs == rhs);
        lhs.setTimeZone(QTimeZone("Europe/Berlin"), {{2020, 02, 29}, {0, 0}});
        QVERIFY(lhs == rhs);
        QVERIFY(rhs == lhs);

        rhs.setTimeZone(QTimeZone("Europe/Vilnius"), {{2020, 02, 29}, {0, 0}});
        QCOMPARE(lhs == rhs, false);
        QCOMPARE(rhs == lhs, false);

        rhs.setTimeZone(QTimeZone("Europe/Brussels"), {{2020, 02, 29}, {0, 0}});
        QCOMPARE(lhs == rhs, true);
        QCOMPARE(rhs == lhs, true);
    }

    void testTzDiff()
    {
        LocationInformation l;
        l.setTimeZone(QTimeZone("Europe/Berlin"), {{2020, 02, 29}, {0, 0}});
        QCOMPARE(l.timeZoneDiffers(), false);
        QCOMPARE(l.timeZoneOffsetDelta(), 0);
        l.setTimeZone(QTimeZone("Europe/Vilnius"), {{2020, 02, 29}, {0, 0}});
        QCOMPARE(l.timeZoneDiffers(), true);
        QCOMPARE(l.timeZoneOffsetDelta(), 3600);
        QVERIFY(!l.timeZoneName().isEmpty());
        l.setTimeZone(QTimeZone("Europe/Berlin"), {{2020, 02, 29}, {0, 0}});
        QCOMPARE(l.timeZoneDiffers(), true);
        QCOMPARE(l.timeZoneOffsetDelta(), -3600);
        QVERIFY(!l.timeZoneName().isEmpty());
    }

    void testBug461438()
    {
        // UK -> BE transition - https://bugs.kde.org/461438
        LocationInformation info;
        info.setIsoCode(QStringLiteral("GB"));
        info.setIsoCode(QStringLiteral("BE"));
        QVERIFY(info.drivingSideDiffers());
        QCOMPARE(info.drivingSide(), KItinerary::KnowledgeDb::DrivingSide::Right);
        QCOMPARE(info.powerPlugCompatibility(), LocationInformation::Incompatible);
        QVERIFY(info.currencyDiffers());
        QCOMPARE(info.currencyCode(), QLatin1StringView("EUR"));
    }

    void testDstChange()
    {
        LocationInformation l;
        l.setTimeZone(QTimeZone("Europe/Berlin"), QDateTime{{2024, 3, 29}, {2, 0}, QTimeZone("Europe/Berlin")});
        l.setTimeZone(QTimeZone("Europe/Berlin"), QDateTime{{2024, 3, 31}, {3, 0}, QTimeZone("Europe/Berlin")});
        QCOMPARE(l.timeZoneDiffers(), true);
        QCOMPARE(l.dstDiffers(), true);
        QCOMPARE(l.isDst(), true);
        QCOMPARE(l.timeZoneOffsetDelta(), 3600);
        l.setTimeZone(QTimeZone("Europe/Berlin"), QDateTime{{2024, 10, 27}, {2, 0}, QTimeZone("Europe/Berlin")});
        QCOMPARE(l.timeZoneDiffers(), true);
        QCOMPARE(l.dstDiffers(), true);
        QCOMPARE(l.isDst(), false);
        QCOMPARE(l.timeZoneOffsetDelta(), -3600);
    }
};

QTEST_GUILESS_MAIN(LocationInformationTest)

#include "locationinformationtest.moc"
