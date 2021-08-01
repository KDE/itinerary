/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <applicationcontroller.h>
#include <reservationmanager.h>
#include <pkpassmanager.h>

#include <KItinerary/Flight>
#include <KItinerary/Reservation>

#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>
#include <KItinerary/Place>
#include <KItinerary/Visit>

using namespace KItinerary;

class ReservationManagerTest : public QObject
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

    void clearPasses(PkPassManager *mgr)
    {
        for (const auto &id : mgr->passes()) {
            mgr->removePass(id);
        }
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
        QStandardPaths::setTestModeEnabled(true);
    }

    void testOperations()
    {
        ReservationManager mgr;
        clearReservations(&mgr);

        QSignalSpy addSpy(&mgr, &ReservationManager::reservationAdded);
        QVERIFY(addSpy.isValid());
        QSignalSpy updateSpy(&mgr, &ReservationManager::reservationChanged);
        QVERIFY(updateSpy.isValid());
        QSignalSpy rmSpy(&mgr, &ReservationManager::reservationRemoved);
        QVERIFY(rmSpy.isValid());

        QVERIFY(mgr.batches().empty());
        mgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/4U8465-v1.json")));

        auto res = mgr.batches();
        QCOMPARE(res.size(), 1);
        const auto &resId = res[0];
        QVERIFY(!resId.isEmpty());

        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(addSpy.at(0).at(0).toString(), resId);
        QVERIFY(updateSpy.isEmpty());
        QVERIFY(!mgr.reservation(resId).isNull());

        mgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/4U8465-v2.json")));
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(mgr.batches().size(), 1);
        QCOMPARE(updateSpy.at(0).at(0).toString(), resId);
        QVERIFY(mgr.reservation(resId).isValid());

        mgr.removeReservation(resId);
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(rmSpy.size(), 1);
        QCOMPARE(rmSpy.at(0).at(0).toString(), resId);
        QVERIFY(mgr.batches().empty());
        QVERIFY(mgr.reservation(resId).isNull());

        clearReservations(&mgr);
        auto attraction = KItinerary::TouristAttraction();
        attraction.setName(QStringLiteral("Sky Tree"));
        auto visit = KItinerary::TouristAttractionVisit();
        visit.setTouristAttraction(attraction);

        const auto addedResId = mgr.addReservation(QVariant::fromValue(visit));
        QCOMPARE(addedResId, mgr.batches().at(0));

        QCOMPARE(addSpy.size(), 2);
        QCOMPARE(mgr.batches().size(), 1);
        QCOMPARE(addSpy.at(1).at(0).toString(), addedResId);
        QVERIFY(mgr.reservation(addedResId).isValid());
    }

    void testPkPassChanges()
    {
        PkPassManager passMgr;
        clearPasses(&passMgr);

        ReservationManager mgr;
        clearReservations(&mgr);

        ApplicationController ctrl;
        ctrl.setPkPassManager(&passMgr);
        ctrl.setReservationManager(&mgr);

        QSignalSpy addSpy(&mgr, &ReservationManager::reservationAdded);
        QVERIFY(addSpy.isValid());
        QSignalSpy updateSpy(&mgr, &ReservationManager::reservationChanged);
        QVERIFY(updateSpy.isValid());

        QVERIFY(mgr.batches().empty());
        const auto passId = QStringLiteral("pass.booking.kde.org/MTIzNA==");

        passMgr.importPass(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        QCOMPARE(addSpy.size(), 1);
        QVERIFY(updateSpy.isEmpty());

        passMgr.importPass(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v2.pkpass")));
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
    }

    void testBatchPersistence()
    {
        ReservationManager mgr;
        clearReservations(&mgr);

        QCOMPARE(mgr.batches().size(), 0);
        mgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        QCOMPARE(mgr.batches().size(), 2);

        const auto batchId = mgr.batches()[0];
        QVERIFY(!batchId.isEmpty());
        QCOMPARE(mgr.batchForReservation(batchId), batchId);

        const auto l = mgr.reservationsForBatch(batchId);
        QCOMPARE(l.size(), 2);
        QVERIFY(!l.at(0).isEmpty());
        QVERIFY(!l.at(1).isEmpty());
        QVERIFY(l.at(0) != l.at(1));
        QVERIFY(l.at(0) == batchId || l.at(1) == batchId);
        const auto resId = l.at(0) == batchId ? l.at(1) : l.at(0);
        QCOMPARE(mgr.batchForReservation(resId), batchId);
        QCOMPARE(mgr.reservationsForBatch(resId), QStringList());

        // recreating the instance should see the same state
        ReservationManager mgr2;
        QCOMPARE(mgr2.batches().size(), 2);
        QCOMPARE(mgr2.batches()[0], batchId);
        const auto l2 = mgr2.reservationsForBatch(batchId);
        QCOMPARE(l2.size(), 2);
        QVERIFY((l2.at(0) == l.at(0) && l2.at(1) == l.at(1)) || (l2.at(0) == l.at(1) && l2.at(1) == l.at(0)));
    }

    void testBatchOperations()
    {
        ReservationManager mgr;
        clearReservations(&mgr);
        QCOMPARE(mgr.batches().size(), 0);

        QSignalSpy batchAddSpy(&mgr, &ReservationManager::batchAdded);
        QSignalSpy batchChangeSpy(&mgr, &ReservationManager::batchChanged);
        QSignalSpy batchContentSpy(&mgr, &ReservationManager::batchContentChanged);
        QSignalSpy batchRenameSpy(&mgr, &ReservationManager::batchRenamed);
        QSignalSpy batchRemovedSpy(&mgr, &ReservationManager::batchRemoved);

        mgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        QCOMPARE(batchAddSpy.size(), 2);
        QCOMPARE(batchChangeSpy.size(), 2);
        QCOMPARE(batchRenameSpy.size(), 0);
        QCOMPARE(batchRemovedSpy.size(), 0);

        const auto batchId = batchAddSpy.at(0).at(0).toString();
        const auto l = mgr.reservationsForBatch(batchId);
        QCOMPARE(l.size(), 2);

        // changing secondary reservation triggers batch update
        const auto secId = l.at(0) == batchId ? l.at(1) : l.at(0);
        QCOMPARE(mgr.batchForReservation(secId), batchId);
        auto res = mgr.reservation(secId).value<FlightReservation>();
        auto flight = res.reservationFor().value<Flight>();
        flight.setArrivalTerminal(QStringLiteral("foo"));
        res.setReservationFor(flight);

        batchAddSpy.clear();
        batchChangeSpy.clear();
        mgr.updateReservation(secId, res);
        QCOMPARE(batchAddSpy.size(), 0);
        QCOMPARE(batchChangeSpy.size(), 0);
        QCOMPARE(batchContentSpy.size(), 1);
        QCOMPARE(batchRenameSpy.size(), 0);
        QCOMPARE(batchRemovedSpy.size(), 0);
        QCOMPARE(batchContentSpy.at(0).at(0), batchId);

        // de-batching by update, moving to new batch
        flight.setDepartureTime(flight.departureTime().addYears(1));
        res.setReservationFor(flight);

        batchAddSpy.clear();
        batchContentSpy.clear();
        mgr.updateReservation(secId, res);
        QCOMPARE(batchAddSpy.size(), 1);
        QCOMPARE(batchChangeSpy.size(), 1);
        QCOMPARE(batchContentSpy.size(), 0);
        QCOMPARE(batchRenameSpy.size(), 0);
        QCOMPARE(batchRemovedSpy.size(), 0);
        QCOMPARE(batchAddSpy.at(0).at(0), secId);
        QCOMPARE(batchChangeSpy.at(0).at(0), batchId);

        // re-batching by update, moving to existing batch
        flight.setDepartureTime(flight.departureTime().addYears(-1));
        res.setReservationFor(flight);

        batchAddSpy.clear();
        batchChangeSpy.clear();
        mgr.updateReservation(secId, res);
        QCOMPARE(batchAddSpy.size(), 0);
        QCOMPARE(batchChangeSpy.size(), 1);
        QCOMPARE(batchContentSpy.size(), 0);
        QCOMPARE(batchRenameSpy.size(), 0);
        QCOMPARE(batchRemovedSpy.size(), 1);
        QCOMPARE(batchChangeSpy.at(0).at(0), batchId);
        QCOMPARE(batchRemovedSpy.at(0).at(0), secId);

        QCOMPARE(mgr.reservationsForBatch(batchId).size(), 2);
        QCOMPARE(mgr.batchForReservation(batchId), batchId);
        QCOMPARE(mgr.batchForReservation(secId), batchId);
        QCOMPARE(mgr.hasBatch(batchId), true);
        QCOMPARE(mgr.hasBatch(secId), false);

        // changing primary does update batch
        auto res2 = mgr.reservation(batchId).value<FlightReservation>();
        auto flight2 = res.reservationFor().value<Flight>();
        flight2.setArrivalTerminal(QStringLiteral("bar"));
        res2.setReservationFor(flight2);

        batchChangeSpy.clear();
        batchRemovedSpy.clear();
        mgr.updateReservation(batchId, res2);
        QCOMPARE(batchAddSpy.size(), 0);
        QCOMPARE(batchChangeSpy.size(), 0);
        QCOMPARE(batchContentSpy.size(), 1);
        QCOMPARE(batchRenameSpy.size(), 0);
        QCOMPARE(batchRemovedSpy.size(), 0);
        QCOMPARE(batchContentSpy.at(0).at(0), batchId);

        // de-batch by changing the primary one renames the batch
        flight2.setDepartureTime(flight2.departureTime().addYears(1));
        res2.setReservationFor(flight2);

        batchContentSpy.clear();
        mgr.updateReservation(batchId, res2);
        QCOMPARE(batchAddSpy.size(), 1);
        QCOMPARE(batchChangeSpy.size(), 0);
        QCOMPARE(batchContentSpy.size(), 0);
        QCOMPARE(batchRenameSpy.size(), 1);
        QCOMPARE(batchRemovedSpy.size(), 0);
        QCOMPARE(batchAddSpy.at(0).at(0), batchId);
        QCOMPARE(batchRenameSpy.at(0).at(0), batchId);
        QCOMPARE(batchRenameSpy.at(0).at(1), secId);

        QCOMPARE(mgr.batches().size(), 3);
        QCOMPARE(mgr.reservationsForBatch(batchId), QStringList(batchId));
        QCOMPARE(mgr.reservationsForBatch(secId), QStringList(secId));
        QCOMPARE(mgr.batchForReservation(batchId), batchId);
        QCOMPARE(mgr.batchForReservation(secId), secId);

        // re-batch again, should be same as before
        flight2.setDepartureTime(flight2.departureTime().addYears(-1));
        res2.setReservationFor(flight2);

        batchAddSpy.clear();
        batchRenameSpy.clear();
        mgr.updateReservation(batchId, res2);
        QCOMPARE(batchAddSpy.size(), 0);
        QCOMPARE(batchChangeSpy.size(), 1);
        QCOMPARE(batchContentSpy.size(), 0);
        QCOMPARE(batchRenameSpy.size(), 0);
        QCOMPARE(batchRemovedSpy.size(), 1);
        QCOMPARE(batchChangeSpy.at(0).at(0), secId);
        QCOMPARE(batchRemovedSpy.at(0).at(0), batchId);

        QCOMPARE(mgr.batches().size(), 2);
        QCOMPARE(mgr.reservationsForBatch(secId).size(), 2);
        QCOMPARE(mgr.batchForReservation(batchId), secId);
        QCOMPARE(mgr.batchForReservation(secId), secId);

        // removing primary reservation triggers batch rename
        batchChangeSpy.clear();
        batchRemovedSpy.clear();
        mgr.removeReservation(secId);
        QCOMPARE(batchAddSpy.size(), 0);
        QCOMPARE(batchChangeSpy.size(), 0);
        QCOMPARE(batchRenameSpy.size(), 1);
        QCOMPARE(batchRemovedSpy.size(), 0);
        QCOMPARE(batchRenameSpy.at(0).at(0), secId);
        QCOMPARE(batchRenameSpy.at(0).at(1), batchId);

        QCOMPARE(mgr.batches().size(), 2);
        QCOMPARE(mgr.reservationsForBatch(batchId).size(), 1);
        QCOMPARE(mgr.batchForReservation(batchId), batchId);
        QCOMPARE(mgr.batchForReservation(secId), QString());

        // removing the entire batch
        batchRenameSpy.clear();
        mgr.removeBatch(batchId);
        QCOMPARE(batchAddSpy.size(), 0);
        QCOMPARE(batchChangeSpy.size(), 0);
        QCOMPARE(batchContentSpy.size(), 0);
        QCOMPARE(batchRenameSpy.size(), 0);
        QCOMPARE(batchRemovedSpy.size(), 1);
        QCOMPARE(batchRemovedSpy.at(0).at(0), batchId);

        QCOMPARE(mgr.batches().size(), 1);
        QCOMPARE(mgr.reservationsForBatch(batchId).size(), 0);
        QCOMPARE(mgr.batchForReservation(batchId), QString());
    }

    void testCancellation()
    {
        ReservationManager mgr;
        clearReservations(&mgr);
        QCOMPARE(mgr.batches().size(), 0);
        mgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));
        QCOMPARE(mgr.batches().size(), 11);

        auto res = mgr.reservation(mgr.batches().at(0));
        QCOMPARE(JsonLd::convert<Reservation>(res).reservationStatus(), Reservation::ReservationConfirmed);
        res = mgr.reservation(mgr.batches().at(10));
        QVERIFY(JsonLd::canConvert<Reservation>(res));
        QCOMPARE(JsonLd::convert<Reservation>(res).reservationStatus(), Reservation::ReservationConfirmed);

        QSignalSpy batchAddSpy(&mgr, &ReservationManager::batchAdded);
        QSignalSpy batchChangeSpy(&mgr, &ReservationManager::batchChanged);
        QSignalSpy batchContentSpy(&mgr, &ReservationManager::batchContentChanged);
        QSignalSpy batchRenameSpy(&mgr, &ReservationManager::batchRenamed);
        QSignalSpy batchRemovedSpy(&mgr, &ReservationManager::batchRemoved);

        mgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/randa2017-flight-cancellation.json")));
        QCOMPARE(mgr.batches().size(), 11);
        QCOMPARE(batchAddSpy.size(), 0);
        QCOMPARE(batchChangeSpy.size(), 0);
        QCOMPARE(batchContentSpy.size(), 2);
        QCOMPARE(batchRenameSpy.size(), 0);
        QCOMPARE(batchRemovedSpy.size(), 0);

        res = mgr.reservation(mgr.batches().at(0));
        QVERIFY(JsonLd::canConvert<Reservation>(res));
        QCOMPARE(JsonLd::convert<Reservation>(res).reservationStatus(), Reservation::ReservationCancelled);
        res = mgr.reservation(mgr.batches().at(10));
        QVERIFY(JsonLd::canConvert<Reservation>(res));
        QCOMPARE(JsonLd::convert<Reservation>(res).reservationStatus(), Reservation::ReservationCancelled);
    }
};

QTEST_GUILESS_MAIN(ReservationManagerTest)

#include "reservationmanagertest.moc"
