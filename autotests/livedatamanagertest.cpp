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

#include <livedata.h>
#include <livedatamanager.h>
#include <reservationmanager.h>

#include <KPublicTransport/Manager>

#include <QJsonDocument>
#include <QJsonObject>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTimeZone>

#define s(x) QStringLiteral(x)

class LiveDataManagerTest : public QObject
{
    Q_OBJECT
private:
    void clearReservations(ReservationManager *mgr)
    {
        const auto batches = mgr->batches(); // copy, as this is getting modified in the process
        for (const auto &id : batches) {
            mgr->removeBatch(id);
        }
        QCOMPARE(mgr->batches().size(), 0);
    }

    QByteArray readFile(const QString &fn)
    {
        QFile f(fn);
        f.open(QFile::ReadOnly);
        return f.readAll();
    }

private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "UTC");
        QStandardPaths::setTestModeEnabled(true);
    }

    void testPersistence()
    {
        LiveData::clearStorage();
        QCOMPARE(LiveData::listAll(), std::vector<QString>());

        {
            LiveData ld;
            ld.departure.setScheduledDepartureTime({{2017, 9, 10}, {11, 0}});
            ld.departureTimestamp = {{ 2017, 1, 1} , {0, 0}};
            ld.store(s("testId"), LiveData::Departure);
        }

        QCOMPARE(LiveData::listAll(), std::vector<QString>({ s("testId") }));

        {
            auto ld = LiveData::load(s("testId"));
            QCOMPARE(ld.departure.scheduledDepartureTime(), QDateTime({2017, 9, 10}, {11, 0}));
            QCOMPARE(ld.departureTimestamp, QDateTime({2017, 1, 1}, {0, 0}));
            QVERIFY(!ld.arrivalTimestamp.isValid());
            ld.departure = {};
            ld.store(s("testId"), LiveData::AllTypes);
        }

        QCOMPARE(LiveData::listAll(), std::vector<QString>());
    }

    void testLiveData()
    {
        ReservationManager resMgr;
        clearReservations(&resMgr);
        LiveData::clearStorage();

        LiveDataManager ldm;
        ldm.setPollingEnabled(false); // we don't want to trigger network requests here
        QVERIFY(ldm.publicTransportManager());
        ldm.m_unitTestTime = QDateTime({2017, 9, 10}, {12, 0}, QTimeZone("Europe/Zurich")); // that's in the middle of the first train leg
        ldm.setReservationManager(&resMgr);

        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));
        QCOMPARE(resMgr.batches().size(), 11);

        const auto flight = resMgr.batches()[0];
        QVERIFY(!ldm.isRelevant(flight));
        QVERIFY(ldm.hasDeparted(flight, resMgr.reservation(flight)));
        QVERIFY(ldm.hasArrived(flight, resMgr.reservation(flight)));

        const auto trainLeg1 = resMgr.batches()[1];
        QVERIFY(ldm.isRelevant(trainLeg1));
        QVERIFY(ldm.hasDeparted(trainLeg1, resMgr.reservation(trainLeg1)));
        QVERIFY(!ldm.hasArrived(trainLeg1, resMgr.reservation(trainLeg1)));

        const auto trainLeg2 = resMgr.batches()[2];
        QVERIFY(ldm.isRelevant(trainLeg2));
        QVERIFY(!ldm.hasDeparted(trainLeg2, resMgr.reservation(trainLeg2)));
        QVERIFY(!ldm.hasArrived(trainLeg2, resMgr.reservation(trainLeg2)));

        QCOMPARE(ldm.nextPollTimeForReservation(flight), std::numeric_limits<int>::max());
        QCOMPARE(ldm.nextPollTimeForReservation(trainLeg1), 0); // no current data available, so we want to poll ASAP
        QCOMPARE(ldm.nextPollTimeForReservation(trainLeg2), 0);
        QCOMPARE(ldm.nextPollTime(), 0);

        const auto leg1Arr = KPublicTransport::Stopover::fromJson(QJsonDocument::fromJson(readFile(s(SOURCE_DIR "/data/livedata/randa2017-leg1-arrival.json"))).object());
        ldm.stopoverQueryFinished({ leg1Arr }, LiveData::Arrival, trainLeg1);
        QCOMPARE(ldm.arrival(trainLeg1).arrivalDelay(), 2);
        QCOMPARE(ldm.nextPollTimeForReservation(trainLeg1), 15 * 60 * 1000); // 15 min in msecs

        // verify this was persisted
        {
            LiveDataManager ldm2;
            QCOMPARE(ldm.arrival(trainLeg1).arrivalDelay(), 2);
        }

        // failed lookups are recorded to avoid a polling loop
        ldm.stopoverQueryFinished({ leg1Arr }, LiveData::Departure, trainLeg2);
        ldm.stopoverQueryFinished({ leg1Arr }, LiveData::Arrival, trainLeg2);
        QCOMPARE(ldm.departure(trainLeg2).stopPoint().isEmpty(), true);
        QCOMPARE(ldm.nextPollTimeForReservation(trainLeg2), 15 * 60 * 1000);
    }
};

QTEST_GUILESS_MAIN(LiveDataManagerTest)

#include "livedatamanagertest.moc"
