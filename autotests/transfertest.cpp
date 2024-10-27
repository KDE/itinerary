/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "applicationcontroller.h"
#include "favoritelocationmodel.h"
#include "importcontroller.h"
#include "livedatamanager.h"
#include "reservationmanager.h"
#include "transfer.h"
#include "transfermanager.h"
#include "tripgroupmanager.h"

#include <QDirIterator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTimeZone>
#include <QtTest/qtest.h>

using namespace Qt::Literals::StringLiterals;

class TransferTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "UTC");
        QStandardPaths::setTestModeEnabled(true);

        qRegisterMetaType<Transfer::Alignment>();
    }

    void testTransferManager()
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
        favLoc.setName(QStringLiteral("name"));
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
        QSignalSpy addSpy(&mgr, &TransferManager::transferAdded);
        QSignalSpy changeSpy(&mgr, &TransferManager::transferChanged);
        QSignalSpy removeSpy(&mgr, &TransferManager::transferRemoved);

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        // ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/akademy2017.json")));
        // ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/akademy2018-program.json")));
        ctrl->commitImport(&importer);

        QCOMPARE(addSpy.size() - removeSpy.size(), 3); // to/from home, and one in between

        auto batchId = resMgr.batches().at(0);
        auto transfer = mgr.transfer(batchId, Transfer::Before);
        QCOMPARE(transfer.state(), Transfer::Pending);
        QCOMPARE(transfer.anchorTime(), QDateTime({2017, 9, 10}, {6, 15}, QTimeZone("Europe/Berlin")));
        QCOMPARE(transfer.anchorTimeDelta(), 5400);
        QCOMPARE(transfer.alignment(), Transfer::Before);
        QCOMPARE(transfer.reservationId(), batchId);
        QVERIFY(transfer.from().hasCoordinate());
        QCOMPARE(transfer.from().latitude(), 52.51860f);
        QCOMPARE(transfer.from().longitude(), 13.37630f);
        QVERIFY(transfer.to().hasCoordinate());
        QCOMPARE(transfer.to().name(), QLatin1StringView("Berlin Tegel"));
        QVERIFY(!mgr.canAddTransfer(batchId, Transfer::Before));

        transfer = mgr.transfer(batchId, Transfer::After);
        QCOMPARE(transfer.state(), Transfer::UndefinedState);
        QVERIFY(!mgr.canAddTransfer(batchId, Transfer::After));

        // verify persistence
        TransferManager mgr2;
        transfer = mgr2.transfer(batchId, Transfer::Before);
        QCOMPARE(transfer.state(), Transfer::Pending);
        QCOMPARE(transfer.anchorTime(), QDateTime({2017, 9, 10}, {6, 15}, QTimeZone("Europe/Berlin")));
        QCOMPARE(transfer.alignment(), Transfer::Before);
        QCOMPARE(transfer.reservationId(), batchId);
        QVERIFY(transfer.hasLocations());
        QVERIFY(transfer.from().hasCoordinate());
        QCOMPARE(transfer.from().latitude(), 52.51860f);
        QCOMPARE(transfer.from().longitude(), 13.37630f);
        QVERIFY(transfer.to().hasCoordinate());
        QCOMPARE(transfer.to().name(), QLatin1StringView("Berlin Tegel"));

        // operations
        addSpy.clear();
        changeSpy.clear();
        removeSpy.clear();

        KPublicTransport::Journey jny;
        KPublicTransport::JourneySection section;
        section.setScheduledDepartureTime(QDateTime({2017, 9, 10}, {5, 0}, QTimeZone("Europe/Berlin")));
        section.setScheduledArrivalTime(QDateTime({2017, 9, 10}, {5, 30}, QTimeZone("Europe/Berlin")));
        jny.setSections({section});
        mgr.setJourneyForTransfer(transfer, jny);
        QCOMPARE(addSpy.size(), 0);
        QCOMPARE(changeSpy.size(), 1);
        QCOMPARE(removeSpy.size(), 0);

        transfer = mgr.transfer(batchId, Transfer::Before);
        QCOMPARE(transfer.state(), Transfer::Selected);
        QCOMPARE(transfer.journey().sections().size(), 1);
        QCOMPARE(transfer.journey().scheduledArrivalTime(), QDateTime({2017, 9, 10}, {5, 30}, QTimeZone("Europe/Berlin")));
        QCOMPARE(transfer.anchorTimeDelta(), 45 * 60);

        addSpy.clear();
        changeSpy.clear();
        removeSpy.clear();

        mgr.discardTransfer(transfer);
        QCOMPARE(addSpy.size(), 0);
        QCOMPARE(changeSpy.size(), 0);
        QCOMPARE(removeSpy.size(), 1);
        transfer = mgr.transfer(batchId, Transfer::Before);
        QCOMPARE(transfer.state(), Transfer::Discarded);
        QVERIFY(mgr.canAddTransfer(batchId, Transfer::Before));

        transfer = mgr.addTransfer(batchId, Transfer::Before);
        QCOMPARE(transfer.state(), Transfer::Pending);
        QCOMPARE(transfer.anchorTime(), QDateTime({2017, 9, 10}, {6, 15}, QTimeZone("Europe/Berlin")));
        QCOMPARE(transfer.alignment(), Transfer::Before);
        QCOMPARE(transfer.reservationId(), batchId);
        QVERIFY(transfer.hasLocations());
        QVERIFY(!mgr.canAddTransfer(batchId, Transfer::Before));
    }

    void testTransfer_data()
    {
        QTest::addColumn<QString>("input");
        QDirIterator it(QLatin1StringView(SOURCE_DIR "/data/transfer/"), {"*.json"_L1});
        while (it.hasNext()) {
            it.next();
            const auto baseName = it.fileInfo().baseName();
            QTest::newRow(baseName.toUtf8().constData()) << it.fileInfo().absoluteFilePath();
        }
    }

    static QJsonObject transferToJson(const Transfer &t)
    {
        auto obj = Transfer::toJson(t);
        obj.remove("reservationId"_L1); // random
        return obj;
    }

    void testTransfer()
    {
        QFETCH(QString, input);

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
        favLoc.setName(QStringLiteral("name"));
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
        importer.importFromUrl(QUrl::fromLocalFile(input));
        ctrl->commitImport(&importer);

        const auto batchIds = resMgr.batches();
        QJsonArray result;
        for (const auto &batchId : batchIds) {
            QJsonObject res;
            if (auto t = mgr.transfer(batchId, Transfer::Before); t.state() == Transfer::Pending) {
                res["before"_L1] = transferToJson(t);
            }
            if (auto t = mgr.transfer(batchId, Transfer::After); t.state() == Transfer::Pending) {
                res["after"_L1] = transferToJson(t);
            }
            result.push_back(res);
        }

        QFile refFile(input + ".ref"_L1);
        QVERIFY(refFile.open(QFile::ReadOnly));
        const auto ref = QJsonDocument::fromJson(refFile.readAll()).array();

        QVERIFY(Test::compareJson(refFile.fileName(), result, ref));

        // batch changes should not change transfers
        QSignalSpy addSpy(&mgr, &TransferManager::transferAdded);
        QSignalSpy changeSpy(&mgr, &TransferManager::transferChanged);
        QSignalSpy removeSpy(&mgr, &TransferManager::transferRemoved);

        for (const auto &batchId : batchIds) {
            Q_EMIT resMgr.batchChanged(batchId);
            Q_EMIT resMgr.batchContentChanged(batchId);
        }
        result = {};
        for (const auto &batchId : batchIds) {
            QJsonObject res;
            if (auto t = mgr.transfer(batchId, Transfer::Before); t.state() == Transfer::Pending) {
                res["before"_L1] = transferToJson(t);
            }
            if (auto t = mgr.transfer(batchId, Transfer::After); t.state() == Transfer::Pending) {
                res["after"_L1] = transferToJson(t);
            }
            result.push_back(res);
        }

        QVERIFY(Test::compareJson(refFile.fileName(), result, ref));
        QCOMPARE(addSpy.size(), 0);
        QCOMPARE(changeSpy.size(), 0);
        QCOMPARE(removeSpy.size(), 0);
    }
};

QTEST_GUILESS_MAIN(TransferTest)

#include "transfertest.moc"
