/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "timelinedelegatecontroller.h"
#include "applicationcontroller.h"
#include "livedatamanager.h"
#include "reservationmanager.h"
#include "transfermanager.h"

#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/Journey>
#include <KPublicTransport/JourneyRequest>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJSValue>
#include <QUrl>
#include <QQmlEngine>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTimeZone>

using namespace KItinerary;

class TimelineDelegateControllerTest : public QObject
{
    Q_OBJECT
private:
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

    void testEmptyController()
    {
        TimelineDelegateController controller;
        QCOMPARE(controller.isCurrent(), false);
        QCOMPARE(controller.progress(), 0.0f);
        QCOMPARE(controller.effectiveEndTime(), QDateTime());
        QCOMPARE(controller.isLocationChange(), false);
        QCOMPARE(controller.isPublicTransport(), false);
        QVERIFY(!controller.journeyRequestFull().isValid());
        QVERIFY(!controller.journeyRequestOne().isValid());
        QCOMPARE(controller.isCanceled(), false);

        controller.setBatchId(QStringLiteral("foo"));
        QCOMPARE(controller.isCurrent(), false);
        QCOMPARE(controller.progress(), 0.0f);
        QCOMPARE(controller.effectiveEndTime(), QDateTime());
        QCOMPARE(controller.isLocationChange(), false);
        QCOMPARE(controller.isPublicTransport(), false);
        QCOMPARE(controller.isCanceled(), false);

        ReservationManager mgr;
        controller.setReservationManager(&mgr);
        QCOMPARE(controller.isCurrent(), false);
        QCOMPARE(controller.progress(), 0.0f);
        QCOMPARE(controller.effectiveEndTime(), QDateTime());
        QCOMPARE(controller.isLocationChange(), false);
        QCOMPARE(controller.isPublicTransport(), false);
        QCOMPARE(controller.isCanceled(), false);
        QCOMPARE(controller.documentIds(), QStringList());
    }

    void testProgress()
    {
        ReservationManager mgr;
        Test::clearAll(&mgr);

        TrainTrip trip;
        trip.setTrainNumber(QStringLiteral("TGV 1235"));
        trip.setDepartureTime(QDateTime::currentDateTime().addDays(-1));
        TrainReservation res;
        res.setReservationNumber(QStringLiteral("XXX007"));
        res.setReservationFor(trip);

        TimelineDelegateController controller;
        QSignalSpy currentSpy(&controller, &TimelineDelegateController::currentChanged);
        controller.setReservationManager(&mgr);

        mgr.addReservation(res);
        QCOMPARE(mgr.batches().size(), 1);
        const auto batchId = mgr.batches().at(0);

        controller.setBatchId(batchId);
        QCOMPARE(controller.isCurrent(), false);
        QCOMPARE(controller.progress(), 0.0f);
        QCOMPARE(controller.isLocationChange(), true);
        QCOMPARE(controller.isPublicTransport(), true);
        QCOMPARE(controller.isCanceled(), false);

        trip.setArrivalTime(QDateTime::currentDateTime().addDays(1));
        res.setReservationFor(trip);
        mgr.updateReservation(batchId, res);

        QCOMPARE(controller.isCurrent(), true);
        QCOMPARE(controller.progress(), 0.5f);
        QCOMPARE(controller.effectiveEndTime(), trip.arrivalTime());
        QCOMPARE(currentSpy.size(), 1);
    }

    void testPreviousLocation()
    {
        ReservationManager mgr;
        Test::clearAll(&mgr);

        TimelineDelegateController controller;
        controller.setReservationManager(&mgr);

        {
            TrainTrip trip;
            trip.setTrainNumber(QStringLiteral("TGV 1235"));
            trip.setDepartureTime(QDateTime::currentDateTime().addDays(2));
            TrainReservation res;
            res.setReservationNumber(QStringLiteral("XXX007"));
            res.setReservationFor(trip);
            mgr.addReservation(res);
        }

        QCOMPARE(mgr.batches().size(), 1);
        const auto batchId = mgr.batches().at(0);

        controller.setBatchId(batchId);
        QCOMPARE(controller.previousLocation(), QVariant());

        TrainStation arrStation;
        arrStation.setName(QStringLiteral("My Station"));
        TrainTrip prevTrip;
        prevTrip.setTrainNumber(QStringLiteral("ICE 1234"));
        prevTrip.setDepartureTime(QDateTime::currentDateTime().addDays(1));
        prevTrip.setArrivalTime(QDateTime::currentDateTime().addDays(1));
        prevTrip.setArrivalStation(arrStation);
        TrainReservation prevRes;
        prevRes.setReservationNumber(QStringLiteral("XXX007"));
        prevRes.setReservationFor(prevTrip);

        QSignalSpy changeSpy(&controller, &TimelineDelegateController::previousLocationChanged);
        mgr.addReservation(prevRes);

        QCOMPARE(changeSpy.size(), 1);
        QVERIFY(!controller.previousLocation().isNull());
        QCOMPARE(controller.previousLocation().value<TrainStation>().name(), QLatin1StringView("My Station"));
    }

    void testJourneyRequest()
    {
        ReservationManager mgr;
        PkPassManager pkPassMgr;
        Test::clearAll(&mgr);
        LiveDataManager ldm;
        ldm.setPkPassManager(&pkPassMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&mgr);
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));

        TimelineDelegateController controller;
        controller.setReservationManager(&mgr);
        controller.setBatchId(mgr.batches().at(0)); // flight
        controller.setLiveDataManager(&ldm);
        QVERIFY(!controller.journeyRequestFull().isValid());
        QVERIFY(!controller.journeyRequestOne().isValid());

        controller.setBatchId(mgr.batches().at(1)); // first train segment
        QCOMPARE(controller.isLocationChange(), true);
        QCOMPARE(controller.isPublicTransport(), true);

        auto jnyReq = controller.journeyRequestFull();
        QCOMPARE(jnyReq.isValid(), true);
        QCOMPARE(jnyReq.from().name(), QStringLiteral("Zürich Flughafen"));
        QCOMPARE(jnyReq.to().name(), QLatin1StringView("Randa"));
        jnyReq = controller.journeyRequestOne();
        QCOMPARE(jnyReq.isValid(), true);
        QCOMPARE(jnyReq.from().name(), QStringLiteral("Zürich Flughafen"));
        QCOMPARE(jnyReq.to().name(), QLatin1StringView("Visp"));

        controller.setBatchId(mgr.batches().at(2)); // second train segment
        jnyReq = controller.journeyRequestFull();
        QCOMPARE(jnyReq.isValid(), true);
        QCOMPARE(jnyReq.from().name(), QLatin1StringView("Visp"));
        QCOMPARE(jnyReq.to().name(), QLatin1StringView("Randa"));
        jnyReq = controller.journeyRequestOne();
        QCOMPARE(jnyReq.isValid(), true);
        QCOMPARE(jnyReq.from().name(), QLatin1StringView("Visp"));
        QCOMPARE(jnyReq.to().name(), QLatin1StringView("Randa"));
    }

    void testApplyJourney()
    {
        ReservationManager mgr;
        PkPassManager pkPassMgr;
        Test::clearAll(&mgr);
        LiveData::clearStorage();
        LiveDataManager ldm;
        ldm.setPkPassManager(&pkPassMgr);
        ldm.setReservationManager(&mgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&mgr);
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));

        TimelineDelegateController controller;
        controller.setReservationManager(&mgr);
        controller.setLiveDataManager(&ldm);
        controller.setBatchId(mgr.batches().at(mgr.batches().size() - 3)); // begin of the return train trip
        QCOMPARE(controller.isPublicTransport(), true);
        const auto batchCount = mgr.batches().size();

        QSignalSpy addSpy(&mgr, &ReservationManager::batchAdded);
        QSignalSpy updateSpy(&mgr, &ReservationManager::batchChanged);
        QSignalSpy rmSpy(&mgr, &ReservationManager::batchRemoved);

        // apply alternative with 3 segments to test segment insertion
        const auto jny3 = KPublicTransport::Journey::fromJson(QJsonDocument::fromJson(readFile(QLatin1StringView(SOURCE_DIR "/data/publictransport/randa-zrh-3-sections.json"))).object());
        controller.applyJourney(QVariant::fromValue(jny3), true);
        QCOMPARE(mgr.batches().size(), batchCount + 1);
        QCOMPARE(addSpy.size(), 3);
        QCOMPARE(updateSpy.size(), 0); // as we move beyond other elements, we get add/remove rather than updated here
        QCOMPARE(rmSpy.size(), 2);
        QCOMPARE(LiveData::listAll().size(), 3);

        // apply alternative with 2 segments to test segment removal
        controller.setBatchId(mgr.batches().at(mgr.batches().size() - 3)); // begin of the new 3 segment train trip
        addSpy.clear();
        updateSpy.clear();
        rmSpy.clear();
        const auto jny2 = KPublicTransport::Journey::fromJson(QJsonDocument::fromJson(readFile(QLatin1StringView(SOURCE_DIR "/data/publictransport/randa-zrh-2-sections.json"))).object());
        controller.applyJourney(QVariant::fromValue(jny2), true);
        QCOMPARE(mgr.batches().size(), batchCount);
        QCOMPARE(addSpy.size(), 2);
        QCOMPARE(updateSpy.size(), 0);
        QCOMPARE(rmSpy.size(), 3);
        QCOMPARE(LiveData::listAll().size(), 2);
    }

    void testCancel()
    {
        ReservationManager mgr;
        Test::clearAll(&mgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&mgr);
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/timeline/flight-cancelation.json")));

        TimelineDelegateController controller;
        controller.setReservationManager(&mgr);
        controller.setBatchId(mgr.batches().at(0));
        QCOMPARE(controller.isCanceled(), true);
        QCOMPARE(controller.connectionWarning(), false);
    }

    void testMapArgs()
    {
        ReservationManager mgr;
        PkPassManager pkPassMgr;
        Test::clearAll(&mgr);
        LiveData::clearStorage();
        LiveDataManager ldm;
        ldm.setPkPassManager(&pkPassMgr);
        ldm.setReservationManager(&mgr);
        TransferManager trfMgr;
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&mgr);
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));

        TimelineDelegateController controller;
        controller.setReservationManager(&mgr);
        controller.setLiveDataManager(&ldm);
        controller.setTransferManager(&trfMgr);

        QQmlEngine engine;
        QQmlEngine::setObjectOwnership(&controller, QQmlEngine::CppOwnership);
        engine.newQObject(&controller);

        // workaround for pre-KF5I18nLocaleData code for region lookups still in use on the CI
        const auto isRegion = [](const QString &region, QLatin1StringView ref) {
            if (region.size() == 2) {
                return ref.startsWith(region);
            }
            return region == ref;
        };

        controller.setBatchId(mgr.batches().at(0)); // flight
        auto args = controller.departureMapArguments().toVariant().toMap();
        QCOMPARE(args.value(QLatin1StringView("departureGateName")).toString(), QLatin1StringView("A10"));
        QCOMPARE(args.value(QLatin1StringView("endTime")).toDateTime(), QDateTime({2017, 9, 10}, {6, 45}, QTimeZone("Europe/Berlin")));
        QCOMPARE(args.value(QLatin1StringView("placeName")).toString(), QLatin1StringView("Berlin Tegel"));
        QVERIFY(isRegion(args.value(QLatin1StringView("region")).toString(), QLatin1StringView("DE-BE")));
        QCOMPARE(args.value(QLatin1StringView("timeZone")), QLatin1StringView("Europe/Berlin"));

        args = controller.arrivalMapArguments().toVariant().toMap();
        // arrivalGate property doesn't exist
        QCOMPARE(args.value(QLatin1StringView("beginTime")).toDateTime(), QDateTime({2017, 9, 10}, {8, 15}, QTimeZone("Europe/Zurich")));
        QCOMPARE(args.value(QLatin1StringView("departurePlatformMode")).toInt(), 1);
        QCOMPARE(args.value(QLatin1StringView("departurePlatformName")).toString(), QLatin1StringView("3"));
        QCOMPARE(args.value(QLatin1StringView("endTime")).toDateTime(), QDateTime({2017, 9, 10}, {11, 40}, QTimeZone("Europe/Zurich")));
        QCOMPARE(args.value(QLatin1StringView("placeName")).toString(), QStringLiteral("Zürich"));
        QVERIFY(isRegion(args.value(QLatin1StringView("region")).toString(), QLatin1StringView("CH-ZH")));
        QCOMPARE(args.value(QLatin1StringView("timeZone")), QLatin1StringView("Europe/Zurich"));

        controller.setBatchId(mgr.batches().at(1)); // first train leg
        args = controller.departureMapArguments().toVariant().toMap();
        // arrivalGate property doesn't exist
        QCOMPARE(args.value(QLatin1StringView("beginTime")).toDateTime(), QDateTime({2017, 9, 10}, {8, 15}, QTimeZone("Europe/Zurich")));
        QCOMPARE(args.value(QLatin1StringView("departurePlatformMode")).toInt(), 1);
        QCOMPARE(args.value(QLatin1StringView("departurePlatformName")).toString(), QLatin1StringView("3"));
        QCOMPARE(args.value(QLatin1StringView("endTime")).toDateTime(), QDateTime({2017, 9, 10}, {11, 40}, QTimeZone("Europe/Zurich")));
        QCOMPARE(args.value(QLatin1StringView("placeName")).toString(), QStringLiteral("Zürich Flughafen"));
        QVERIFY(isRegion(args.value(QLatin1StringView("region")).toString(), QLatin1StringView("CH-ZH")));
        QCOMPARE(args.value(QLatin1StringView("timeZone")), QLatin1StringView("Europe/Zurich"));

        args = controller.arrivalMapArguments().toVariant().toMap();
        QCOMPARE(args.value(QLatin1StringView("arrivalPlatformMode")).toInt(), 1);
        QCOMPARE(args.value(QLatin1StringView("arrivalPlatformName")).toString(), QLatin1StringView("7"));
        QCOMPARE(args.value(QLatin1StringView("beginTime")).toDateTime(), QDateTime({2017, 9, 10}, {14, 2}, QTimeZone("Europe/Zurich")));
        QCOMPARE(args.value(QLatin1StringView("departurePlatformMode")).toInt(), 1);
        QCOMPARE(args.value(QLatin1StringView("departurePlatformName")).toString(), QLatin1StringView("3"));
        QCOMPARE(args.value(QLatin1StringView("endTime")).toDateTime(), QDateTime({2017, 9, 10}, {14, 8}, QTimeZone("Europe/Zurich")));
        QCOMPARE(args.value(QLatin1StringView("placeName")).toString(), QLatin1StringView("Visp"));
        QVERIFY(isRegion(args.value(QLatin1StringView("region")).toString(), QLatin1StringView("CH-VS")));
        QCOMPARE(args.value(QLatin1StringView("timeZone")), QLatin1StringView("Europe/Zurich"));

        controller.setBatchId(mgr.batches().at(2)); // final train leg
        args = controller.departureMapArguments().toVariant().toMap();
        QCOMPARE(args.value(QLatin1StringView("arrivalPlatformMode")).toInt(), 1);
        QCOMPARE(args.value(QLatin1StringView("arrivalPlatformName")).toString(), QLatin1StringView("7"));
        QCOMPARE(args.value(QLatin1StringView("beginTime")).toDateTime(), QDateTime({2017, 9, 10}, {14, 2}, QTimeZone("Europe/Zurich")));
        QCOMPARE(args.value(QLatin1StringView("departurePlatformMode")).toInt(), 1);
        QCOMPARE(args.value(QLatin1StringView("departurePlatformName")).toString(), QLatin1StringView("3"));
        QCOMPARE(args.value(QLatin1StringView("endTime")).toDateTime(), QDateTime({2017, 9, 10}, {14, 8}, QTimeZone("Europe/Zurich")));
        QCOMPARE(args.value(QLatin1StringView("placeName")).toString(), QLatin1StringView("Visp"));
        QVERIFY(isRegion(args.value(QLatin1StringView("region")).toString(), QLatin1StringView("CH-VS")));
        QCOMPARE(args.value(QLatin1StringView("timeZone")), QLatin1StringView("Europe/Zurich"));

        args = controller.arrivalMapArguments().toVariant().toMap();
        QCOMPARE(args.value(QLatin1StringView("arrivalPlatformMode")).toInt(), 1);
        QCOMPARE(args.value(QLatin1StringView("arrivalPlatformName")).toString(), QString());
        QCOMPARE(args.value(QLatin1StringView("beginTime")).toDateTime(), QDateTime({2017, 9, 10}, {14, 53}, QTimeZone("Europe/Zurich")));
        QCOMPARE(args.value(QLatin1StringView("placeName")).toString(), QLatin1StringView("Randa"));
        QVERIFY(isRegion(args.value(QLatin1StringView("region")).toString(), QLatin1StringView("CH-VS")));
        QCOMPARE(args.value(QLatin1StringView("timeZone")), QLatin1StringView("Europe/Zurich"));

        controller.setBatchId(mgr.batches().at(3)); // event
        args = controller.mapArguments().toVariant().toMap();
        QCOMPARE(args.value(QLatin1StringView("placeName")).toString(), QLatin1StringView("Haus Randa"));
        QVERIFY(isRegion(args.value(QLatin1StringView("region")).toString(), QLatin1StringView("CH-VS")));
        auto coord = args.value(QLatin1StringView("coordinate")).value<QPointF>();
        QVERIFY(!coord.isNull());
        QCOMPARE(args.value(QLatin1StringView("beginTime")).toDateTime(), QDateTime({2017, 9, 10}, {14, 53}, QTimeZone("Europe/Zurich")));
        QVERIFY(!args.value(QLatin1StringView("endTime")).toDateTime().isValid());

        controller.setBatchId(mgr.batches().at(4)); // accommodation
        args = controller.mapArguments().toVariant().toMap();
        QCOMPARE(args.value(QLatin1StringView("placeName")).toString(), QLatin1StringView("Haus Randa"));
        QVERIFY(isRegion(args.value(QLatin1StringView("region")).toString(), QLatin1StringView("CH-VS")));
        coord = args.value(QLatin1StringView("coordinate")).value<QPointF>();
        QVERIFY(!coord.isNull());
        QCOMPARE(args.value(QLatin1StringView("beginTime")).toDateTime(), QDateTime({2017, 9, 10}, {14, 53}, QTimeZone("Europe/Zurich")));
        QCOMPARE(args.value(QLatin1StringView("endTime")).toDateTime(), QDateTime({2017, 9, 15}, {14, 54}, QTimeZone("Europe/Zurich")));

        controller.setBatchId(mgr.batches().at(7)); // food
        args = controller.mapArguments().toVariant().toMap();
        QCOMPARE(args.value(QLatin1StringView("placeName")).toString(), QLatin1StringView("Raclette"));
        QVERIFY(isRegion(args.value(QLatin1StringView("region")).toString(), QLatin1StringView("CH-VS")));
        QCOMPARE(args.value(QLatin1StringView("beginTime")).toDateTime(), QDateTime({2017, 9, 14}, {0, 0}, QTimeZone("Europe/Zurich")));
        QCOMPARE(args.value(QLatin1StringView("endTime")).toDateTime(), QDateTime({2017, 9, 15}, {0, 0}, QTimeZone("Europe/Zurich")));

        controller.setBatchId(mgr.batches().at(9)); // final return leg
        args = controller.arrivalMapArguments().toVariant().toMap();
        QCOMPARE(args.value(QLatin1StringView("arrivalPlatformMode")).toInt(), 1);
        QCOMPARE(args.value(QLatin1StringView("arrivalPlatformName")).toString(), QLatin1StringView("2"));
        QCOMPARE(args.value(QLatin1StringView("beginTime")).toDateTime(), QDateTime({2017, 9, 15}, {18, 16}, QTimeZone("Europe/Zurich")));
        QCOMPARE(args.value(QLatin1StringView("departureGateName")).toString(), QLatin1StringView("52"));
        QCOMPARE(args.value(QLatin1StringView("endTime")).toDateTime(), QDateTime({2017, 9, 15}, {20, 50}, QTimeZone("Europe/Zurich")));
        QCOMPARE(args.value(QLatin1StringView("placeName")).toString(), QStringLiteral("Zürich Flughafen"));
        QVERIFY(isRegion(args.value(QLatin1StringView("region")).toString(), QLatin1StringView("CH-ZH")));
        QCOMPARE(args.value(QLatin1StringView("timeZone")), QLatin1StringView("Europe/Zurich"));
        coord = args.value(QLatin1StringView("coordinate")).value<QPointF>();
        QVERIFY(!coord.isNull());

        controller.setBatchId(mgr.batches().at(10)); // return flight
        args = controller.departureMapArguments().toVariant().toMap();
        QCOMPARE(args.value(QLatin1StringView("arrivalPlatformMode")).toInt(), 1);
        QCOMPARE(args.value(QLatin1StringView("arrivalPlatformName")).toString(), QLatin1StringView("2"));
        QCOMPARE(args.value(QLatin1StringView("beginTime")).toDateTime(), QDateTime({2017, 9, 15}, {18, 16}, QTimeZone("Europe/Zurich")));
        QCOMPARE(args.value(QLatin1StringView("departureGateName")).toString(), QLatin1StringView("52"));
        QCOMPARE(args.value(QLatin1StringView("endTime")).toDateTime(), QDateTime({2017, 9, 15}, {20, 50}, QTimeZone("Europe/Zurich")));
        QCOMPARE(args.value(QLatin1StringView("placeName")).toString(), QStringLiteral("Zürich"));
        QVERIFY(isRegion(args.value(QLatin1StringView("region")).toString(), QLatin1StringView("CH-ZH")));
        QCOMPARE(args.value(QLatin1StringView("timeZone")), QLatin1StringView("Europe/Zurich"));

        // single static element, see 44ed4fbe96d45a60b33e2bf784196489197d7dcc
        Test::clearAll(&mgr);
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/haus-randa-v1.json")));
        QCOMPARE(mgr.batches().size(), 1);
        controller.setBatchId(mgr.batches().at(0));
        args = controller.mapArguments().toVariant().toMap();
        QCOMPARE(args.value(QLatin1StringView("placeName")).toString(), QLatin1StringView("Haus Randa"));
        QCOMPARE(args.value(QLatin1StringView("beginTime")).toDateTime(), QDateTime({2017, 9, 10}, {0, 0}, QTimeZone("Europe/Zurich")));
        QCOMPARE(args.value(QLatin1StringView("endTime")).toDateTime(), QDateTime({2017, 9, 16}, {0, 0}, QTimeZone("Europe/Zurich")));
    }
};

QTEST_GUILESS_MAIN(TimelineDelegateControllerTest)

#include "timelinedelegatecontrollertest.moc"
