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

#include <KPublicTransport/Departure>
#include <KPublicTransport/Journey>
#include <KPublicTransport/Line>
#include <KPublicTransport/NavitiaParser>

#include <QFile>
#include <QTest>
#include <QTimeZone>

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
            QCOMPARE(journey.departureTime(), QDateTime({2018, 12, 2}, {22, 4, 51}));
            QEXPECT_FAIL("", "tz propagation not implemented yet", Continue);
            QCOMPARE(journey.departureTime().timeZone().id(), "Europe/Paris");
            QCOMPARE(journey.arrivalTime(), QDateTime({2018, 12, 2}, {23, 0, 15}));
            QEXPECT_FAIL("", "tz propagation not implemented yet", Continue);
            QCOMPARE(journey.arrivalTime().timeZone().id(), "Europe/Paris");
            QCOMPARE(journey.duration(), 3324);

            auto sec = journey.sections()[0];
            QCOMPARE(sec.mode(), KPublicTransport::JourneySection::Walking);
            QEXPECT_FAIL("", "tz propagation not implemented yet", Continue);
            QCOMPARE(sec.from().timeZone().id(), "Europe/Paris");

            sec = journey.sections()[1];
            QCOMPARE(sec.mode(), KPublicTransport::JourneySection::PublicTransport);
            QCOMPARE(sec.departureTime(), QDateTime({2018, 12, 2}, {22, 6}, QTimeZone("Europe/Paris")));
            QCOMPARE(sec.departureTime().timeZone().id(), "Europe/Paris");
            QCOMPARE(sec.arrivalTime(), QDateTime({2018, 12, 2}, {22, 41}, QTimeZone("Europe/Paris")));
            QCOMPARE(sec.arrivalTime().timeZone().id(), "Europe/Paris");
            QCOMPARE(sec.duration(), 2100);
            QCOMPARE(sec.from().name(), QStringLiteral("Aéroport CDG 2 TGV (Le Mesnil-Amelot)"));
            QCOMPARE(sec.from().latitude(), 49.0047f);
            QCOMPARE(sec.from().timeZone().id(), "Europe/Paris");
            QCOMPARE(sec.to().name(), QStringLiteral("Châtelet les Halles (Paris)"));
            QCOMPARE(sec.to().longitude(), 2.34701f);
            QCOMPARE(sec.to().timeZone().id(), "Europe/Paris");
            QCOMPARE(sec.route().line().name(), QStringLiteral("B"));
            QCOMPARE(sec.route().line().mode(), KPublicTransport::Line::RapidTransit);
            QCOMPARE(sec.route().line().modeString(), QStringLiteral("RER"));
            QCOMPARE(sec.route().line().color(), QColor(123, 163, 220));
            QCOMPARE(sec.route().line().textColor(), QColor(255, 255, 255));

            sec = journey.sections()[2];
            QCOMPARE(sec.mode(), KPublicTransport::JourneySection::Transfer);

            sec = journey.sections()[3];
            QCOMPARE(sec.mode(), KPublicTransport::JourneySection::Waiting);

            sec = journey.sections()[4];
            QCOMPARE(sec.departureTime(), QDateTime({2018, 12, 2}, {22, 49}, QTimeZone("Europe/Paris")));
            QCOMPARE(sec.arrivalTime(), QDateTime({2018, 12, 2}, {22, 51}, QTimeZone("Europe/Paris")));
            QCOMPARE(sec.duration(), 120);
            QCOMPARE(sec.route().line().name(), QStringLiteral("A"));
            QCOMPARE(sec.route().line().color(), QColor(QStringLiteral("#D1302F")));
            QCOMPARE(sec.route().line().textColor(), QColor(255, 255, 255));
            QCOMPARE(sec.from().name(), QStringLiteral("Châtelet les Halles (Paris)"));
            QCOMPARE(sec.to().name(), QStringLiteral("Gare de Lyon RER A (Paris)"));
        }

        {
            const auto journey = res[1];
            QCOMPARE(journey.sections().size(), 6);

            auto sec = journey.sections()[1];
            QCOMPARE(sec.route().line().name(), QStringLiteral("B"));
            QCOMPARE(sec.route().line().mode(), KPublicTransport::Line::RapidTransit);
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
            QCOMPARE(sec.route().line().modeString(), QStringLiteral("Bus"));
            QCOMPARE(sec.route().line().mode(), KPublicTransport::Line::Bus);
        }

        {
            const auto journey = res[3];
            QCOMPARE(journey.sections().size(), 3);

            auto sec = journey.sections()[1];
            QCOMPARE(sec.route().line().name(), QStringLiteral("DIRECT 4"));
            QCOMPARE(sec.route().line().mode(), KPublicTransport::Line::Bus);
            QCOMPARE(sec.route().line().modeString(), QStringLiteral("Bus"));
        }
    }

    void testParseDepartures()
    {
        const auto res = KPublicTransport::NavitiaParser::parseDepartures(readFile(SOURCE_DIR "/data/navitia/departure-response.json"));
        QCOMPARE(res.size(), 10);

        {
            const auto departure = res[0];
            QCOMPARE(departure.scheduledTime(), QDateTime({2018, 12, 10}, {17, 17}, QTimeZone("Europe/Paris")));
            QCOMPARE(departure.stopPoint().name(), QStringLiteral("Gare de Lyon - Diderot (Paris)"));
            QCOMPARE(departure.route().direction(), QStringLiteral("Porte de la Chapelle (Paris)"));
            QCOMPARE(departure.route().line().mode(), KPublicTransport::Line::Bus);
            QCOMPARE(departure.route().line().name(), QStringLiteral("65"));
            QCOMPARE(departure.route().line().color(), QColor(0x00, 0x8b, 0x5a));
        }

        {
            const auto departure = res[3];
            QCOMPARE(departure.scheduledTime(), QDateTime({2018, 12, 10}, {17, 19}, QTimeZone("Europe/Paris")));
            QCOMPARE(departure.stopPoint().name(), QStringLiteral("Gare de Lyon RER D (Paris)"));
            QCOMPARE(departure.route().direction(), QStringLiteral("Gare de Villiers le Bel Gonesse Arnouville (Arnouville)"));
            QCOMPARE(departure.route().line().mode(), KPublicTransport::Line::RapidTransit);
            QCOMPARE(departure.route().line().modeString(), QStringLiteral("RER"));
            QCOMPARE(departure.route().line().name(), QStringLiteral("D"));
            QCOMPARE(departure.route().line().color(), QColor(0x5E, 0x96, 0x20));
        }
    }
};

QTEST_GUILESS_MAIN(NavitiaParserTest)

#include "navitiaparsertest.moc"
