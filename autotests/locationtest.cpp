/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#include <KPublicTransport/Location>

#include <QTest>

#define s(x) QStringLiteral(x)

using namespace KPublicTransport;

class LocationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLocationNameCompare_data()
    {
        QTest::addColumn<QString>("lhs");
        QTest::addColumn<QString>("rhs");

        QTest::newRow("identical") << s("Berlin Hbf") << s("Berlin Hbf");

        QTest::newRow("case folding") << s("Berlin Hbf") << s("Berlin HBF");

        QTest::newRow("separators1") << s("Berlin Hauptbahnhof") << s("Berlin, Hauptbahnhof");
        QTest::newRow("separators2") << s("Berlin Hauptbahnhof") << s("Berlin (Hauptbahnhof)");
        QTest::newRow("separators3") << s("Berlin, Hauptbahnhof") << s("Berlin (Hauptbahnhof)");
        QTest::newRow("separators4") << s("Paris-Gare-de-Lyon") << s("Paris Gare de Lyon");

        QTest::newRow("abbreviations1") << s("Berlin Hbf") << s("Berlin Hauptbahnhof");
        QTest::newRow("abbreviations2") << s("Amsterdam Cs") << s("Amsterdam Centraal");

        QTest::newRow("extras1") << s("S+U Berlin Hbf") << s("Berlin Hauptbahnhof");
        QTest::newRow("extras2") << s("Berlin Schönefeld Flughafen (S)") << s("S Berlin Schönefeld Flughafen");
        QTest::newRow("extras3") << s("Paris Gare de Lyon RER") << s("Paris Gare de Lyon");

        QTest::newRow("station1") << s("Boissy-St-Léger") << s("Gare de Boissy-St-Léger");
        QTest::newRow("station2") << s("Flughafen Wien") << s("Flughafen Wien Bahnhof");
        QTest::newRow("station3") << s("Berlin Schönefeld Flughafen Bhf") << s("Berlin Schönefeld Flughafen");

        QTest::newRow("duplicates1") << s("Paris Gare de Lyon (Paris)") << s("Paris Gare de Lyon");

        QTest::newRow("normalization1") << s("Boissy-St-Léger (Boissy-Saint-Léger)") << s("Boissy St Léger");

        QTest::newRow("order1") << s("Paris Gare de Lyon") << s("Gare de Lyon (Paris)");

        QTest::newRow("localization1") << s("Berlin Flughafen Schönefeld (Airport)") << s("Berlin Flughafen Schönefeld Bhf");

        QTest::newRow("diacritic1") << s("Berlin Flughafen Schonefeld") << s("Berlin Flughafen Schönefeld");
        QTest::newRow("diacritic2") << s("Berlin Flughafen Schoenefeld") << s("Berlin Flughafen Schönefeld");
        QTest::newRow("diacritic3") << s("København H") << s("Koebenhavn H");
    }

    void testLocationNameCompare()
    {
        QFETCH(QString, lhs);
        QFETCH(QString, rhs);
        Location l, r;
        l.setName(lhs);
        r.setName(rhs);
        QVERIFY(Location::isSame(l, r));
    }
};

QTEST_GUILESS_MAIN(LocationTest)

#include "locationtest.moc"
