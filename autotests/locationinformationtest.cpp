/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <locationinformation.h>

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
        lhs.setTimeZone(QTimeZone("Europe/Berlin"));
        QVERIFY(lhs == rhs);
        QVERIFY(rhs == lhs);

        rhs.setTimeZone(QTimeZone("Europe/Vilnius"));
        QCOMPARE(lhs == rhs, false);
        QCOMPARE(rhs == lhs, false);

        rhs.setTimeZone(QTimeZone("Europe/Brussels"));
        QCOMPARE(lhs == rhs, true);
        QCOMPARE(rhs == lhs, true);
    }

    void testTzDiff()
    {
        LocationInformation l;
        l.setTimeZone(QTimeZone("Europe/Berlin"));
        QCOMPARE(l.timeZoneDiffers(), false);
        l.setTimeZone(QTimeZone("Europe/Vilnius"));
        QCOMPARE(l.timeZoneDiffers(), true);
        QVERIFY(!l.timeZoneName().isEmpty());
    }
};

QTEST_GUILESS_MAIN(LocationInformationTest)

#include "locationinformationtest.moc"
