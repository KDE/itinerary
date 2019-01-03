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
};

QTEST_GUILESS_MAIN(HafasParserTest)

#include "hafasparsertest.moc"
