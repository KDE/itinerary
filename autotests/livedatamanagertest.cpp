/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"
#include "mocknetworkaccessmanager.h"

#include "livedata.h"
#include "livedatamanager.h"
#include "applicationcontroller.h"
#include "reservationmanager.h"

#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/Manager>

#include <QJsonDocument>
#include <QJsonObject>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTimeZone>

#define s(x) QStringLiteral(x)

using namespace KItinerary;

static MockNetworkAccessManager s_nam;
static QNetworkAccessManager* namFactory() { return &s_nam; }

class LiveDataManagerTest : public QObject
{
    Q_OBJECT
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
        PkPassManager pkPassMgr;
        Test::clearAll(&resMgr);
        QSignalSpy resChangeSpy(&resMgr, &ReservationManager::batchContentChanged);
        LiveData::clearStorage();

        LiveDataManager ldm;
        ldm.setPkPassManager(&pkPassMgr);
        QSignalSpy arrivalUpdateSpy(&ldm, &LiveDataManager::arrivalUpdated);
        QSignalSpy departureUpdateSpy(&ldm, &LiveDataManager::departureUpdated);
        ldm.setPollingEnabled(false); // we don't want to trigger network requests here
        QVERIFY(ldm.publicTransportManager());
        ldm.m_unitTestTime = QDateTime({2017, 9, 10}, {12, 0}, QTimeZone("Europe/Zurich")); // that's in the middle of the first train leg
        ldm.setReservationManager(&resMgr);

        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
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
        QTest::qWait(0);
        QCOMPARE(ldm.nextPollTime(), 0);
        QCOMPARE(resMgr.reservation(trainLeg1).value<TrainReservation>().reservationFor().value<TrainTrip>().arrivalStation().address().addressLocality(), QString());

        const auto leg1Arr = KPublicTransport::Stopover::fromJson(QJsonDocument::fromJson(Test::readFile(s(SOURCE_DIR "/data/livedata/randa2017-leg1-arrival.json"))).object());
        ldm.stopoverQueryFinished({ leg1Arr }, LiveData::Arrival, trainLeg1);
        QCOMPARE(arrivalUpdateSpy.size(), 1);
        QCOMPARE(arrivalUpdateSpy.at(0).at(0).toString(), trainLeg1);
        QCOMPARE(departureUpdateSpy.size(), 0);
        QCOMPARE(ldm.arrival(trainLeg1).arrivalDelay(), 2);
        QCOMPARE(ldm.nextPollTimeForReservation(trainLeg1), 15 * 60 * 1000); // 15 min in msecs
        // reservation was updated with additional location data
        QCOMPARE(resChangeSpy.size(), 1);
        QCOMPARE(resChangeSpy.at(0).at(0).toString(), trainLeg1);
        QCOMPARE(resMgr.reservation(trainLeg1).value<TrainReservation>().reservationFor().value<TrainTrip>().arrivalStation().address().addressLocality(), QLatin1StringView("Visp"));

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

    void testPkPassUpdate()
    {
        PkPassManager pkPassMgr;
        QSignalSpy passUpdateSpy(&pkPassMgr, &PkPassManager::passUpdated);
        pkPassMgr.setNetworkAccessManagerFactory(namFactory);
        Test::clearAll(&pkPassMgr);
        ReservationManager resMgr;
        Test::clearAll(&resMgr);

        LiveData::clearStorage();
        LiveDataManager ldm;
        ldm.setPkPassManager(&pkPassMgr);
        ldm.setPollingEnabled(true);
        ldm.m_unitTestTime = QDateTime({2023, 7, 14}, {15, 0, 0}, QTimeZone("Europe/Berlin"));
        ldm.setReservationManager(&resMgr);

        auto ctrl = Test::makeAppController();
        ctrl->setPkPassManager(&pkPassMgr);
        ctrl->setReservationManager(&resMgr);
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/updateable-boardingpass.pkpass")));

        QCOMPARE(resMgr.batches().size(), 1);
        const auto resId = resMgr.batches()[0];
        QCOMPARE(ldm.isRelevant(resId), true);
        QCOMPARE(ldm.nextPollTimeForReservation(resId), 0);
        QTest::qWait(0);

        QCOMPARE(pkPassMgr.passes().size(), 1);
        const auto pass = pkPassMgr.pass(pkPassMgr.passes()[0]);
        QVERIFY(pass);
        QVERIFY(PkPassManager::canUpdate(pass));

        QCOMPARE(s_nam.requests.size(), 1);
        QTest::qWait(0); // download failed
        QCOMPARE(passUpdateSpy.size(), 0);
        QVERIFY(ldm.nextPollTimeForReservation(resId) > 0);
        QVERIFY(ldm.nextPollTimeForReservation(resId) <= 30000);
        QVERIFY(ldm.pollCooldown(resId) > 0);
        QVERIFY(ldm.pollCooldown(resId) <= 30000);
        QVERIFY(ldm.nextPollTime() > 0);
        QVERIFY(ldm.nextPollTime() <= 30000);

        ldm.m_unitTestTime = QDateTime({2023, 7, 14}, {15, 5, 0}, QTimeZone("Europe/Berlin"));
        QCOMPARE(ldm.nextPollTimeForReservation(resId), 0);
        QCOMPARE(ldm.pollCooldown(resId), 0);
        QCOMPARE(ldm.nextPollTime(), 0);
    }
};

QTEST_GUILESS_MAIN(LiveDataManagerTest)

#include "livedatamanagertest.moc"
