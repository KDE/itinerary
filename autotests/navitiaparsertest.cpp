/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

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

#include <KPublicTransport/Journey>
#include <KPublicTransport/NavitiaParser>

#include <QFile>
#include <QTest>

class NavitiaParserTest : public QObject
{
    Q_OBJECT
private:
    QByteArray readFile(const char *fn)
    {
        QFile f(QString::fromUtf8(fn));
        f.open(QFile::ReadOnly);
        return f.readAll();
    }

private slots:
    void initTestCase()
    {
        qputenv("TZ", "UTC");
    }

    void testParseJourneys()
    {
        const auto res = KPublicTransport::NavitiaParser::parseJourneys(readFile(SOURCE_DIR "/data/navitia/journey-response.json"));
        QCOMPARE(res.size(), 4);

        {
            const auto journey = res[0];
            QCOMPARE(journey.sections().size(), 6);

            auto sec = journey.sections()[1];
            QCOMPARE(sec.departureTime(), QDateTime({2018, 12, 2}, {22, 06}));
            QCOMPARE(sec.arrivalTime(), QDateTime({2018, 12, 2}, {22, 41}));
            QCOMPARE(sec.from().latitude(), 49.0047f);
            QCOMPARE(sec.to().longitude(), 2.34701f);
            QCOMPARE(sec.route().line().name(), QStringLiteral("B"));

            sec = journey.sections()[4];
            QCOMPARE(sec.departureTime(), QDateTime({2018, 12, 2}, {22, 49}));
            QCOMPARE(sec.arrivalTime(), QDateTime({2018, 12, 2}, {22, 51}));
            QCOMPARE(sec.route().line().name(), QStringLiteral("A"));
        }

        {
            const auto journey = res[1];
            QCOMPARE(journey.sections().size(), 6);

            auto sec = journey.sections()[1];
            QCOMPARE(sec.route().line().name(), QStringLiteral("B"));
            sec = journey.sections()[4];
            QCOMPARE(sec.route().line().name(), QStringLiteral("65"));
        }

        {
            const auto journey = res[2];
            QCOMPARE(journey.sections().size(), 6);

            auto sec = journey.sections()[1];
            QCOMPARE(sec.route().line().name(), QStringLiteral("B"));
            sec = journey.sections()[4];
            QCOMPARE(sec.route().line().name(), QStringLiteral("91"));
        }

        {
            const auto journey = res[3];
            QCOMPARE(journey.sections().size(), 3);

            auto sec = journey.sections()[1];
            QCOMPARE(sec.route().line().name(), QStringLiteral("DIRECT 4"));
        }
    }
};

QTEST_GUILESS_MAIN(NavitiaParserTest)

#include "navitiaparsertest.moc"
