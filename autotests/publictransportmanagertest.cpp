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

#include <KPublicTransport/DepartureReply>
#include <KPublicTransport/DepartureRequest>
#include <KPublicTransport/Manager>
#include <KPublicTransport/JourneyRequest>
#include <KPublicTransport/JourneyReply>

#include <QSignalSpy>
#include <QTest>

using namespace KPublicTransport;

class PublicTransportManagerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "UTC");
    }

    void testQueryJourney()
    {
        Manager mgr;
        auto reply = mgr.queryJourney({});
        QSignalSpy spy(reply, &Reply::finished);
        QVERIFY(spy.wait(100));
        QCOMPARE(spy.size(), 1);
        delete reply;
    }

    void testQueryDepartures()
    {
        Manager mgr;
        auto reply = mgr.queryDeparture({});
        QSignalSpy spy(reply, &Reply::finished);
        QVERIFY(spy.wait(100));
        QCOMPARE(spy.size(), 1);
        delete reply;
    }
};

QTEST_GUILESS_MAIN(PublicTransportManagerTest)

#include "publictransportmanagertest.moc"
