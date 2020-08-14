/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <timelinedelegatecontroller.h>
#include <livedatamanager.h>
#include <reservationmanager.h>

#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/Journey>
#include <KPublicTransport/JourneyRequest>

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

using namespace KItinerary;

class TimelineDelegateControllerTest : public QObject
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

    void testEmptyController()
    {
        TimelineDelegateController controller;
        QCOMPARE(controller.isCurrent(), false);
        QCOMPARE(controller.progress(), 0.0f);
        QCOMPARE(controller.effectiveEndTime(), QDateTime());
        QCOMPARE(controller.isLocationChange(), false);
        QCOMPARE(controller.isPublicTransport(), false);
        QVERIFY(!controller.journeyRequest().isValid());
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
    }

    void testProgress()
    {
        ReservationManager mgr;
        clearReservations(&mgr);

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
        clearReservations(&mgr);

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
        QCOMPARE(controller.previousLocation().value<TrainStation>().name(), QLatin1String("My Station"));
    }

    void testJourneyRequest()
    {
        ReservationManager mgr;
        clearReservations(&mgr);
        mgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));

        TimelineDelegateController controller;
        controller.setReservationManager(&mgr);
        controller.setBatchId(mgr.batches().at(0)); // flight
        QVERIFY(!controller.journeyRequest().isValid());

        controller.setBatchId(mgr.batches().at(1)); // first train segment
        QCOMPARE(controller.isLocationChange(), true);
        QCOMPARE(controller.isPublicTransport(), true);

        auto jnyReq = controller.journeyRequest();
        QCOMPARE(jnyReq.isValid(), true);
        QCOMPARE(jnyReq.from().name(), QStringLiteral("Zürich Flughafen"));
        QCOMPARE(jnyReq.to().name(), QLatin1String("Randa"));

        controller.setBatchId(mgr.batches().at(2)); // second train segment
        jnyReq = controller.journeyRequest();
        QCOMPARE(jnyReq.isValid(), true);
        QCOMPARE(jnyReq.from().name(), QLatin1String("Visp"));
        QCOMPARE(jnyReq.to().name(), QLatin1String("Randa"));
    }

    void testApplyJourney()
    {
        ReservationManager mgr;
        clearReservations(&mgr);
        LiveData::clearStorage();
        LiveDataManager ldm;
        ldm.setReservationManager(&mgr);
        mgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));

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
        const auto jny3 = KPublicTransport::Journey::fromJson(QJsonDocument::fromJson(readFile(QLatin1String(SOURCE_DIR "/data/publictransport/randa-zrh-3-sections.json"))).object());
        controller.applyJourney(QVariant::fromValue(jny3));
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
        const auto jny2 = KPublicTransport::Journey::fromJson(QJsonDocument::fromJson(readFile(QLatin1String(SOURCE_DIR "/data/publictransport/randa-zrh-2-sections.json"))).object());
        controller.applyJourney(QVariant::fromValue(jny2));
        QCOMPARE(mgr.batches().size(), batchCount);
        QCOMPARE(addSpy.size(), 2);
        QCOMPARE(updateSpy.size(), 0);
        QCOMPARE(rmSpy.size(), 3);
        QCOMPARE(LiveData::listAll().size(), 2);
    }

    void testCancel()
    {
        ReservationManager mgr;
        clearReservations(&mgr);
        mgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/timeline/flight-cancelation.json")));

        TimelineDelegateController controller;
        controller.setReservationManager(&mgr);
        controller.setBatchId(mgr.batches().at(0));
        QCOMPARE(controller.isCanceled(), true);
        QCOMPARE(controller.connectionWarning(), false);
    }
};

QTEST_GUILESS_MAIN(TimelineDelegateControllerTest)

#include "timelinedelegatecontrollertest.moc"
