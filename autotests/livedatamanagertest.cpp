/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mocknetworkaccessmanager.h"
#include "testhelper.h"

#include "applicationcontroller.h"
#include "importcontroller.h"
#include "livedata.h"
#include "livedatamanager.h"
#include "reservationmanager.h"
#include "timelinedelegatecontroller.h"

#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/Manager>

#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTimeZone>
#include <QtTest/qtest.h>

#define s(x) QStringLiteral(x)

using namespace Qt::Literals;
using namespace KItinerary;

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
            ld.trip.setMode(KPublicTransport::JourneySection::PublicTransport);
            ld.trip.setScheduledDepartureTime({{2017, 9, 10}, {11, 0}});
            ld.journeyTimestamp = {{2017, 1, 1}, {0, 0}};
            ld.store(s("testId"));
        }

        QCOMPARE(LiveData::listAll(), std::vector<QString>({s("testId")}));

        {
            auto ld = LiveData::load(s("testId"));
            QCOMPARE(ld.journey().scheduledDepartureTime(), QDateTime({2017, 9, 10}, {11, 0}));
            QCOMPARE(ld.journeyTimestamp, QDateTime({2017, 1, 1}, {0, 0}));
            ld.trip = {};
            ld.store(s("testId"));
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
        QSignalSpy journeyUpdateSpy(&ldm, &LiveDataManager::journeyUpdated);
        ldm.setPollingEnabled(false); // we don't want to trigger network requests here
        QVERIFY(ldm.publicTransportManager());
        ldm.m_unitTestTime = QDateTime({2017, 9, 10}, {12, 0}, QTimeZone("Europe/Zurich")); // that's in the middle of the first train leg
        ldm.setReservationManager(&resMgr);

        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);
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

        const auto leg1Jny = KPublicTransport::JourneySection::fromJson(QJsonDocument::fromJson(Test::readFile(s(SOURCE_DIR "/data/livedata/randa2017-leg1-journey.json"))).object());
        ldm.applyJourney(trainLeg1, leg1Jny);
        QCOMPARE(journeyUpdateSpy.size(), 1);
        QCOMPARE(journeyUpdateSpy.at(0).at(0).toString(), trainLeg1);
        QCOMPARE(ldm.arrival(trainLeg1).arrivalDelay(), 2);
        QCOMPARE(ldm.nextPollTimeForReservation(trainLeg1), 15 * 60 * 1000); // 15 min in msecs
        // reservation was updated with additional location data
        QCOMPARE(resChangeSpy.size(), 1);
        QCOMPARE(resChangeSpy.at(0).at(0).toString(), trainLeg1);
        QCOMPARE(resMgr.reservation(trainLeg1).value<TrainReservation>().reservationFor().value<TrainTrip>().arrivalStation().address().addressLocality(), "Visp"_L1);
        auto &ld = ldm.data(trainLeg1);
        QCOMPARE(ld.departureIndex, 0);
        QCOMPARE(ld.arrivalIndex, 1);
        QCOMPARE(ld.trip.intermediateStops().size(), 0);

        // verify this was persisted
        {
            LiveDataManager ldm2;
            QCOMPARE(ldm.arrival(trainLeg1).arrivalDelay(), 2);
            ld = ldm2.data(trainLeg1);
            QCOMPARE(ld.departureIndex, 0);
            QCOMPARE(ld.arrivalIndex, 1);
        }

        // applying the same again does not update the reservation
        ldm.applyJourney(trainLeg1, leg1Jny);
        QCOMPARE(journeyUpdateSpy.size(), 2);
        QCOMPARE(ldm.arrival(trainLeg1).arrivalDelay(), 2);
        QCOMPARE(resChangeSpy.size(), 1);

        // applying the full trip will not change departure/arrival
        const auto leg1FullJny = KPublicTransport::JourneySection::fromJson(QJsonDocument::fromJson(Test::readFile(s(SOURCE_DIR "/data/livedata/randa2017-leg1-full-journey.json"))).object());
        ldm.applyJourney(trainLeg1, leg1FullJny);
        QCOMPARE(journeyUpdateSpy.size(), 3);
        QCOMPARE(ldm.arrival(trainLeg1).arrivalDelay(), 2);
        QCOMPARE(resChangeSpy.size(), 1);
        ld = ldm.data(trainLeg1);
        QCOMPARE(ld.departure().stopPoint().name(), u"Zürich Flughafen");
        QCOMPARE(ld.arrival().stopPoint().name(), "Visp"_L1);
        QCOMPARE(ld.arrival().scheduledPlatform(), "7"_L1);
        QCOMPARE(ld.arrival().expectedPlatform(), ""_L1);
        QCOMPARE(ld.journey().departure().stopPoint().name(), u"Zürich Flughafen");
        QCOMPARE(ld.journey().arrival().stopPoint().name(), "Visp"_L1);
        QCOMPARE(ld.journey().intermediateStops().size(), 4);
        QCOMPARE(ld.trip.departure().stopPoint().name(), "Romanshorn"_L1);
        QCOMPARE(ld.trip.arrival().stopPoint().name(), "Brig"_L1);
        QCOMPARE(ld.trip.intermediateStops().size(), 10);
        QCOMPARE(ld.departureIndex, 5);
        QCOMPARE(ld.arrivalIndex, 10);

        // applying full journey retains trip data
        const auto leg1PartialJny = KPublicTransport::JourneySection::fromJson(QJsonDocument::fromJson(Test::readFile(s(SOURCE_DIR "/data/livedata/randa2017-leg1-partial-journey.json"))).object());
        ldm.applyJourney(trainLeg1, leg1PartialJny);
        QCOMPARE(journeyUpdateSpy.size(), 4);
        QCOMPARE(ldm.arrival(trainLeg1).arrivalDelay(), 2);
        QCOMPARE(resChangeSpy.size(), 1);
        ld = ldm.data(trainLeg1);
        QCOMPARE(ld.departure().stopPoint().name(), u"Zürich Flughafen"_s);
        QCOMPARE(ld.arrival().stopPoint().name(), "Visp"_L1);
        QCOMPARE(ld.journey().departure().stopPoint().name(), u"Zürich Flughafen"_s);
        QCOMPARE(ld.journey().arrival().stopPoint().name(), "Visp"_L1);
        QCOMPARE(ld.journey().intermediateStops().size(), 4);
        QCOMPARE(ld.arrival().scheduledPlatform(), "7"_L1);
        QCOMPARE(ld.arrival().expectedPlatform(), "6"_L1);
        QCOMPARE(ld.trip.departure().stopPoint().name(), "Romanshorn"_L1);
        QCOMPARE(ld.trip.arrival().stopPoint().name(), "Brig"_L1);
        QCOMPARE(ld.trip.intermediateStops().size(), 10);
        QCOMPARE(ld.departureIndex, 5);
        QCOMPARE(ld.arrivalIndex, 10);

        // applying vehicle layouts
        QCOMPARE(ld.arrival().vehicleLayout().features().size(), 0);
        QCOMPARE(ld.arrival().platformLayout().isEmpty(), true);
        TimelineDelegateController tdc;
        tdc.setBatchId(trainLeg1);
        tdc.setLiveDataManager(&ldm);
        QCOMPARE(tdc.journey().departure().stopPoint().name(), u"Zürich Flughafen"_s);
        QCOMPARE(tdc.journey().arrival().stopPoint().name(), "Visp"_L1);
        const auto arrivalVehicleLayout = KPublicTransport::Stopover::merge(leg1PartialJny.arrival(), KPublicTransport::Stopover::fromJson(QJsonDocument::fromJson(Test::readFile(s(SOURCE_DIR "/data/livedata/randa2017-leg1-arrival-vehicle-layout.json"))).object()));
        QCOMPARE(arrivalVehicleLayout.vehicleLayout().combinedFeatures().size(), 4);
        tdc.setVehicleLayout(arrivalVehicleLayout, true);
        QCOMPARE(journeyUpdateSpy.size(), 5);
        ld = ldm.data(trainLeg1);
        QCOMPARE(ld.departure().stopPoint().name(), u"Zürich Flughafen"_s);
        QCOMPARE(ld.arrival().stopPoint().name(), "Visp"_L1);
        QCOMPARE(ld.arrival().expectedPlatform(), "6"_L1);
        QCOMPARE(ld.arrival().vehicleLayout().combinedFeatures().size(), 4);
        QCOMPARE(ld.arrival().platformLayout().isEmpty(), false);
        QCOMPARE(tdc.tripDepartureIndex(), 5);
        QCOMPARE(tdc.tripArrivalIndex(), 10);

        // applying full journey retains layout information
        ldm.applyJourney(trainLeg1, leg1PartialJny);
        QCOMPARE(journeyUpdateSpy.size(), 6);
        QCOMPARE(ldm.arrival(trainLeg1).arrivalDelay(), 2);
        QCOMPARE(resChangeSpy.size(), 1);
        ld = ldm.data(trainLeg1);
        QCOMPARE(ld.departure().stopPoint().name(), u"Zürich Flughafen");
        QCOMPARE(ld.arrival().stopPoint().name(), "Visp"_L1);
        QCOMPARE(ld.arrival().vehicleLayout().combinedFeatures().size(), 4);
        QCOMPARE(ld.arrival().platformLayout().isEmpty(), false);
        QCOMPARE(ld.journey().departure().stopPoint().name(), u"Zürich Flughafen");
        QCOMPARE(ld.journey().arrival().stopPoint().name(), "Visp"_L1);
        QCOMPARE(ld.journey().intermediateStops().size(), 4);
        QCOMPARE(ld.arrival().expectedPlatform(), "6"_L1);
        QCOMPARE(ld.trip.departure().stopPoint().name(), "Romanshorn"_L1);
        QCOMPARE(ld.trip.arrival().stopPoint().name(), "Brig"_L1);
        QCOMPARE(ld.trip.intermediateStops().size(), 10);
        QCOMPARE(ld.departureIndex, 5);
        QCOMPARE(ld.arrivalIndex, 10);

        // failed lookups are recorded to avoid a polling loop
        ldm.tripQueryFailed(trainLeg2);
        QCOMPARE(ldm.departure(trainLeg2).stopPoint().isEmpty(), true);
        QCOMPARE(ldm.nextPollTimeForReservation(trainLeg2), 15 * 60 * 1000);
    }

    void testPkPassUpdate()
    {
        PkPassManager pkPassMgr;
        QSignalSpy passUpdateSpy(&pkPassMgr, &PkPassManager::passUpdated);
        pkPassMgr.setNetworkAccessManagerFactory([this]() {
            return &m_nam;
        });
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
        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/updateable-boardingpass.pkpass")));
        ctrl->commitImport(&importer);

        QCOMPARE(resMgr.batches().size(), 1);
        const auto resId = resMgr.batches()[0];
        QCOMPARE(ldm.isRelevant(resId), true);
        QCOMPARE(ldm.nextPollTimeForReservation(resId), 0);
        QTest::qWait(1);

        QCOMPARE(pkPassMgr.passes().size(), 1);
        const auto pass = pkPassMgr.pass(pkPassMgr.passes()[0]);
        QVERIFY(pass);
        QVERIFY(PkPassManager::canUpdate(pass));

        QCOMPARE(m_nam.requests.size(), 1);
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

private:
    MockNetworkAccessManager m_nam;
};

QTEST_GUILESS_MAIN(LiveDataManagerTest)

#include "livedatamanagertest.moc"
