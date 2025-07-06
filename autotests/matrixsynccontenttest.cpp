/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "favoritelocationmodel.h"
#include "importcontroller.h"
#include "livedatamanager.h"
#include "matrixsynccontent.h"
#include "matrixsyncstateevent.h"
#include "reservationmanager.h"
#include "transfermanager.h"
#include "tripgroupmanager.h"

#include <KPublicTransport/Journey>

#include <Quotient/events/stateevent.h>

#include <QStandardPaths>
#include <QtTest/qtest.h>

using namespace Qt::Literals;

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

    void testTransfer()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);

        TripGroupManager::clear();
        TripGroupManager tgMgr;
        tgMgr.setReservationManager(&resMgr);

        FavoriteLocationModel favLocModel;
        while (favLocModel.rowCount()) {
            favLocModel.removeLocation(0);
        }
        FavoriteLocation favLoc;
        favLoc.setLatitude(52.51860f);
        favLoc.setLongitude(13.37630f);
        favLoc.setName(u"name"_s);
        favLocModel.setFavoriteLocations({favLoc});
        QCOMPARE(favLocModel.rowCount(), 1);

        LiveDataManager liveDataMgr;

        TransferManager::clear();
        TransferManager mgr;
        mgr.setFavoriteLocationModel(&favLocModel);
        mgr.overrideCurrentDateTime(QDateTime({2017, 1, 1}, {}));
        mgr.setReservationManager(&resMgr);
        mgr.setLiveDataManager(&liveDataMgr);
        tgMgr.setTransferManager(&mgr);

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);

        auto batchId = resMgr.batches().at(0);
        auto transfer = mgr.transfer(batchId, Transfer::Before);
        QCOMPARE(transfer.state(), Transfer::Pending);
        QCOMPARE(transfer.anchorTimeDelta(), 5400);
        QVERIFY(transfer.from().hasCoordinate());
        QCOMPARE(transfer.to().name(), "Berlin Tegel"_L1);

        const auto evBefore = MatrixSyncContent::stateEventForTransfer(batchId, Transfer::Before, &mgr);
        QCOMPARE(evBefore.type(), "org.kde.itinerary.transfer"_L1);
        QCOMPARE(evBefore.stateKey(), batchId + "-BEFORE"_L1);
        QVERIFY(!evBefore.content().isEmpty());

        MatrixSyncContent::readTransfer(evBefore, &mgr);
        transfer = mgr.transfer(batchId, Transfer::Before);
        QCOMPARE(transfer.state(), Transfer::Pending);
        QCOMPARE(transfer.anchorTimeDelta(), 5400);
        QVERIFY(transfer.from().hasCoordinate());
        QCOMPARE(transfer.to().name(), "Berlin Tegel"_L1);

        mgr.removeTransfer(batchId, Transfer::Before);
        transfer = mgr.transfer(batchId, Transfer::Before);
        QCOMPARE(transfer.state(), Transfer::UndefinedState);

        const auto evBeforeEmpty = MatrixSyncContent::stateEventForTransfer(batchId, Transfer::Before, &mgr);
        QCOMPARE(evBeforeEmpty.type(), evBefore.type());
        QCOMPARE(evBeforeEmpty.stateKey(), evBefore.stateKey());

        MatrixSyncContent::readTransfer(evBefore, &mgr);
        transfer = mgr.transfer(batchId, Transfer::Before);
        QCOMPARE(transfer.state(), Transfer::Pending);
        QCOMPARE(transfer.anchorTimeDelta(), 5400);
        QVERIFY(transfer.from().hasCoordinate());
        QCOMPARE(transfer.to().name(), "Berlin Tegel"_L1);

        MatrixSyncContent::readTransfer(evBeforeEmpty, &mgr);
        transfer = mgr.transfer(batchId, Transfer::Before);
        QCOMPARE(transfer.state(), Transfer::UndefinedState);
    }

    void testDocuments()
    {
        DocumentManager docMgr;
        Test::clearAll(&docMgr);

        KItinerary::CreativeWork info;
        info.setEncodingFormat(u"application/json"_s);
        info.setName(u"randa2017.json"_s);
        docMgr.addDocument(u"1234"_s, info, QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json"));

        QCOMPARE(docMgr.documents().size(), 1);

        auto ev = MatrixSyncContent::stateEventForDocument(u"1234"_s, &docMgr);
        QCOMPARE(ev.type(), MatrixSync::DocumentEventType);
        QCOMPARE(ev.stateKey(), "1234"_L1);
        QVERIFY(QFile::exists(ev.fileName()));
        QVERIFY(ev.needsUpload());

        Test::clearAll(&docMgr);
        QVERIFY(!docMgr.hasDocument("1234"_L1));

        ev.setFileName(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json"));
        QVERIFY(!ev.needsDownload());
        MatrixSyncContent::readDocument(ev, &docMgr);
        QVERIFY(docMgr.hasDocument("1234"_L1));

        info = docMgr.documentInfo("1234"_L1).value<KItinerary::CreativeWork>();
        QCOMPARE(info.name(), "randa2017.json"_L1);
        QVERIFY(QFile::exists(docMgr.documentFilePath("1234"_L1)));
    }

    void testPkPass()
    {
        PkPassManager pkPassMgr;
        Test::clearAll(&pkPassMgr);

        const auto id = pkPassMgr.importPass(QUrl::fromLocalFile(QStringLiteral(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        QCOMPARE(pkPassMgr.passes().size(), 1);

        auto ev = MatrixSyncContent::stateEventForPkPass(id);
        QCOMPARE(ev.type(), MatrixSync::PkPassEventType);
        QCOMPARE(ev.stateKey(), id);
        QCOMPARE(ev.needsUpload(), false);
        QVERIFY(ev.content().startsWith("PK"));

        Test::clearAll(&pkPassMgr);
        QVERIFY(!pkPassMgr.hasPass(id));

        MatrixSyncContent::readPkPass(ev, &pkPassMgr);
        QVERIFY(pkPassMgr.hasPass(id));
    }
};

QTEST_GUILESS_MAIN(MatrixSyncContentTest)

#include "matrixsynccontenttest.moc"
