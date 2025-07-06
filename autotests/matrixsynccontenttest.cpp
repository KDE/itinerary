/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "importcontroller.h"
#include "livedatamanager.h"
#include "matrixsynccontent.h"
#include "matrixsyncstateevent.h"
#include "reservationmanager.h"

#include <KPublicTransport/Journey>

#include <Quotient/events/stateevent.h>

#include <QStandardPaths>
#include <QtTest/qtest.h>

class MatrixSyncContentTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testReservation()
    {
        ReservationManager mgr;
        Test::clearAll(&mgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&mgr);

        QCOMPARE(mgr.batches().size(), 0);
        ImportController importer;
        importer.setReservationManager(&mgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(mgr.batches().size(), 2);

        const auto batchId = mgr.batches()[0];
        QCOMPARE(mgr.reservationsForBatch(batchId).size(), 2);
        auto ev = MatrixSyncContent::stateEventForBatch(batchId, &mgr);
        QCOMPARE(ev.type(), "org.kde.itinerary.reservation"_L1);
        QCOMPARE(ev.stateKey(), batchId);
        QVERIFY(!ev.content().isEmpty());

        QCOMPARE(MatrixSyncContent::readBatch(ev, &mgr), batchId);
        QCOMPARE(mgr.batches().size(), 2);
        QCOMPARE(mgr.hasBatch(batchId), true);
        QCOMPARE(mgr.reservationsForBatch(batchId).size(), 2);

        mgr.removeBatch(batchId);
        QCOMPARE(mgr.batches().size(), 1);
        QCOMPARE(mgr.hasBatch(batchId), false);

        QCOMPARE(MatrixSyncContent::readBatch(ev, &mgr), batchId);
        QCOMPARE(mgr.batches().size(), 2);
        QCOMPARE(mgr.hasBatch(batchId), true);
        QCOMPARE(mgr.reservationsForBatch(batchId).size(), 2);

        ev = MatrixSyncContent::stateEventForDeletedBatch(batchId);
        QCOMPARE(ev.type(), "org.kde.itinerary.reservation"_L1);
        QCOMPARE(ev.stateKey(), batchId);
        QVERIFY(ev.content().isEmpty());

        QCOMPARE(MatrixSyncContent::readBatch(ev, &mgr), QString());
        QCOMPARE(mgr.batches().size(), 1);
        QCOMPARE(mgr.hasBatch(batchId), false);
    }

    void testLiveData()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        LiveData::clearStorage();

        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(resMgr.batches().size(), 11);

        PkPassManager pkPassMgr;
        LiveDataManager ldm;
        ldm.setPkPassManager(&pkPassMgr);
        ldm.setPollingEnabled(false); // we don't want to trigger network requests here
        ldm.setReservationManager(&resMgr);

        const auto trainLeg1 = resMgr.batches()[1];
        const auto evEmpty = MatrixSyncContent::stateEventForLiveData(trainLeg1);
        QCOMPARE(evEmpty.type(), "org.kde.itinerary.livedata"_L1);
        QCOMPARE(evEmpty.stateKey(), trainLeg1);

        const auto jny = KPublicTransport::JourneySection::fromJson(QJsonDocument::fromJson(Test::readFile(QLatin1StringView(SOURCE_DIR "/data/livedata/randa2017-leg1-journey.json"))).object());
        ldm.applyJourney(trainLeg1, jny);
        QCOMPARE(ldm.arrival(trainLeg1).arrivalDelay(), 2);

        const auto evData = MatrixSyncContent::stateEventForLiveData(trainLeg1);
        QCOMPARE(evData.type(), "org.kde.itinerary.livedata"_L1);
        QCOMPARE(evData.stateKey(), trainLeg1);
        QVERIFY(!evData.content().isEmpty());

        MatrixSyncContent::readLiveData(evData, &ldm);
        QCOMPARE(ldm.arrival(trainLeg1).arrivalDelay(), 2);

        MatrixSyncContent::readLiveData(evEmpty, &ldm);
        QCOMPARE(ldm.arrival(trainLeg1).arrivalDelay(), 0);

        MatrixSyncContent::readLiveData(evData, &ldm);
        QCOMPARE(ldm.arrival(trainLeg1).arrivalDelay(), 2);
    }
};

QTEST_GUILESS_MAIN(MatrixSyncContentTest)

#include "matrixsynccontenttest.moc"
