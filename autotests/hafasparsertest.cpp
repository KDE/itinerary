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
#include <KPublicTransport/HafasMgateParser>

#include <QFile>
#include <QTest>
#include <QTimeZone>

#define s(x) QStringLiteral(x)

using namespace KPublicTransport;

class HafasParserTest : public QObject
{
    Q_OBJECT
private:
    QByteArray readFile(const char *fn)
    {
        QFile f(QString::fromUtf8(fn));
        f.open(QFile::ReadOnly);
        return f.readAll();
    }

private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "UTC");
    }

    void testParseDepartureError()
    {
        HafasMgateParser p;
        const auto res = p.parseDepartures(readFile(SOURCE_DIR "/data/hafas/stationboard-error-response.json"));
        QVERIFY(res.empty());
        QCOMPARE(p.error(), Reply::NotFoundError);
        QVERIFY(!p.errorMessage().isEmpty());
    }

    void parseDateTime_data()
    {
        QTest::addColumn<QString>("date");
        QTest::addColumn<QString>("time");
        QTest::addColumn<QDateTime>("dt");

        QTest::newRow("empty") << QString() << QString() << QDateTime();
        QTest::newRow("same day") << s("20190105") << s("142100") << QDateTime({2019, 1, 5}, {14, 21});
        QTest::newRow("next day") << s("20190105") << s("01142100") << QDateTime({2019, 1, 6}, {14, 21});
    }

    void parseDateTime()
    {
        QFETCH(QString, date);
        QFETCH(QString, time);
        QFETCH(QDateTime, dt);
        QCOMPARE(HafasMgateParser::parseDateTime(date, time), dt);
    }
};

QTEST_GUILESS_MAIN(HafasParserTest)

#include "hafasparsertest.moc"
